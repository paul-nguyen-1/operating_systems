#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

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
        error("CLIENT: ERROR opening socket");

    struct sockaddr_in serverAddress;
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        fprintf(stderr, "Error: could not contact enc_server on port %s\n", argv[3]);
        exit(2);
    }

    char clientID[] = "enc_client\n";
    if (send(socketFD, clientID, strlen(clientID), 0) < 0)
        error("CLIENT: send error");

    char serverResponse[32];
    memset(serverResponse, '\0', sizeof(serverResponse));
    if (recv(socketFD, serverResponse, sizeof(serverResponse) - 1, 0) < 0)
        error("CLIENT: recv error");
    size_t len = strlen(serverResponse);
    if (len && serverResponse[len - 1] == '\n')
        serverResponse[len - 1] = '\0';

    if (strcmp(serverResponse, "enc_server") != 0)
    {
        fprintf(stderr, "Error: could not contact enc_server on port %s\n", argv[3]);
        close(socketFD);
        exit(2);
    }

    send(socketFD, plaintext, plainTextLength, 0);
    send(socketFD, "\n", 1, 0);
    send(socketFD, key, keyLength, 0);
    send(socketFD, "\n", 1, 0);

    cipherText = malloc(plainTextLength + 2);
    if (!cipherText)
    {
        fprintf(stderr, "enc_client: memory allocation failed\n");
        exit(1);
    }

    memset(cipherText, '\0', plainTextLength + 2);
    if (recv(socketFD, cipherText, plainTextLength + 1, 0) < 0)
        error("CLIENT: recv error");

    printf("%s\n", cipherText);

    close(socketFD);
    return 0;
}
