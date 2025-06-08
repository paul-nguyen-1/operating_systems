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
    memcpy((char *)&address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

static size_t loadFileContent(const char *path, char **buffer)
{
    FILE *filePath = fopen(path, "r");
    if (!filePath)
    {
        fprintf(stderr, "enc_client: cannot open %s\n", path);
        exit(1);
    }

    fseek(filePath, 0, SEEK_END);
    long fileSize = ftell(filePath);
    rewind(filePath);

    *buffer = malloc(fileSize + 1);
    if (!*buffer)
    {
        fprintf(stderr, "enc_client: memory allocation failed\n");
        exit(1);
    }

    size_t length = fread(*buffer, 1, fileSize, filePath);
    if (ferror(filePath))
    {
        fprintf(stderr, "enc_client: read error on %s\n", path);
        exit(1);
    }
    fclose(filePath);

    if (length && (*buffer)[length - 1] == '\n')
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
            fprintf(stderr, "enc_client error: input contains bad characters\n");
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
        {
            continue;
        }

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

static char *readLine(int sockFD)
{
    size_t bufSize = 32;
    char *buffer = malloc(bufSize);
    if (!buffer)
    {
        perror("malloc");
        exit(1);
    }

    size_t i = 0;
    char character;
    while (recv(sockFD, &character, 1, 0) == 1)
    {
        if (character == '\n')
        {
            break;
        }

        if (i >= bufSize - 1)
        {
            bufSize *= 2;
            buffer = realloc(buffer, bufSize);
            if (!buffer)
            {
                perror("realloc");
                exit(1);
            }
        }

        buffer[i++] = character;
    }

    buffer[i] = '\0';
    return buffer;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
        exit(1);
    }

    char *plaintext = NULL, *key = NULL, *cipherText = NULL;
    size_t plainTextLength = loadFileContent(argv[1], &plaintext);
    size_t keyLength = loadFileContent(argv[2], &key);

    validateInput(plaintext);
    validateInput(key);

    if (keyLength < plainTextLength)
    {
        fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
        exit(1);
    }

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0)
    {
        error("CLIENT: ERROR opening socket");
    }

    struct sockaddr_in serverAddress;
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        fprintf(stderr, "Error: could not contact enc_server on port %s\n", argv[3]);
        exit(2);
    }

    char clientID[] = "enc_client\n";
    sendAll(socketFD, clientID, strlen(clientID));

    char *serverResponse = readLine(socketFD);

    if (strcmp(serverResponse, "enc_server") != 0)
    {
        fprintf(stderr, "Error: could not contact enc_server on port %s\n", argv[3]);
        close(socketFD);
        exit(2);
    }

    sendAll(socketFD, plaintext, plainTextLength);
    sendAll(socketFD, "\n", 1);
    sendAll(socketFD, key, keyLength);
    sendAll(socketFD, "\n", 1);

    char *cipherText = readLine(socketFD);
    printf("%s\n", cipherText);
    free(cipherText);

    close(socketFD);
    return 0;
}
