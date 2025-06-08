#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void setupAddressStruct(struct sockaddr_in *address, int portNumber, char *hostname)
{
    memset((char *)address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);

    struct hostent *hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL)
    {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    memcpy((char *)&address->sin_addr.s_addr,
           hostInfo->h_addr_list[0],
           hostInfo->h_length);
}

static size_t loadFileContent(const char *path, char **buffer)
{
    FILE *filePath = fopen(path, "r");
    if (!filePath)
    {
        fprintf(stderr, "dec_client: cannot open %s\n", path);
        exit(1);
    }

    fseek(filePath, 0, SEEK_END);
    long fileSize = ftell(filePath);
    rewind(filePath);

    *buffer = malloc(fileSize + 1);
    if (!*buffer)
    {
        fprintf(stderr, "dec_client: memory allocation failed\n");
        exit(1);
    }

    size_t length = fread(*buffer, 1, fileSize, filePath);
    if (ferror(filePath))
    {
        fprintf(stderr, "dec_client: read error on %s\n", path);
        exit(1);
    }
    fclose(filePath);

    while (length && ((*buffer)[length - 1] == '\n' || (*buffer)[length - 1] == '\r'))
        length--;

    (*buffer)[length] = '\0';
    return length;
}

static void validateInput(const char *text)
{
    while (*text)
    {
        if (*text != ' ' && (*text < 'A' || *text > 'Z'))
        {
            fprintf(stderr, "dec_client error: input contains bad characters\n");
            exit(1);
        }
        text++;
    }
}

static void sendAll(int fd, const char *buffer, size_t length)
{
    while (length)
    {
        ssize_t sent = send(fd, buffer, length, 0);

        if (sent < 0 && errno == EINTR)
            continue;

        if (sent < 0)
        {
            perror("send");
            exit(1);
        }
        if (sent == 0)
        {
            fprintf(stderr, "sendAll: connection closed\n");
            exit(1);
        }
        buffer += sent;
        length -= sent;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "USAGE: %s ciphertext key port\n", argv[0]);
        exit(1);
    }

    char *ciphertext = NULL, *key = NULL, *plaintext = NULL;
    size_t cipherLength = loadFileContent(argv[1], &ciphertext);
    size_t keyLength = loadFileContent(argv[2], &key);

    validateInput(ciphertext);
    validateInput(key);

    if (keyLength < cipherLength)
    {
        fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
        exit(1);
    }

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0)
        error("CLIENT: ERROR opening socket");

    struct sockaddr_in serverAddress;
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    if (connect(socketFD, (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) < 0)
    {
        fprintf(stderr,
                "Error: could not contact dec_server on port %s\n",
                argv[3]);
        exit(2);
    }

    char clientID[] = "dec_client\n";
    if (send(socketFD, clientID, strlen(clientID), 0) < 0)
        error("CLIENT: send error");

    char serverResponse[32];
    memset(serverResponse, '\0', sizeof(serverResponse));
    if (recv(socketFD, serverResponse, sizeof(serverResponse) - 1, 0) < 0)
        error("CLIENT: recv error");

    size_t respLen = strlen(serverResponse);
    if (respLen && serverResponse[respLen - 1] == '\n')
        serverResponse[respLen - 1] = '\0';

    if (strcmp(serverResponse, "dec_server") != 0)
    {
        fprintf(stderr,
                "Error: could not contact dec_server on port %s\n",
                argv[3]);
        close(socketFD);
        exit(2);
    }

    sendAll(socketFD, ciphertext, cipherLength);
    sendAll(socketFD, "\n", 1);
    sendAll(socketFD, key, keyLength);
    sendAll(socketFD, "\n", 1);

    plaintext = malloc(cipherLength + 2);
    if (!plaintext)
    {
        fprintf(stderr, "dec_client: memory allocation failed\n");
        exit(1);
    }
    memset(plaintext, '\0', cipherLength + 2);

    if (recv(socketFD, plaintext, cipherLength + 1, 0) < 0)
        error("CLIENT: recv error");

    printf("%s\n", plaintext);

    close(socketFD);
    return 0;
}
