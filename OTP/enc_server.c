#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void setupAddressStruct(struct sockaddr_in *address, int portNumber)
{
    memset((char *)address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    address->sin_addr.s_addr = INADDR_ANY;
}

static char *readLine(int sockFD)
{
    size_t bufSize = 1024;
    char *buffer = malloc(bufSize);
    if (!buffer)
    {
        fprintf(stderr, "enc_server: memory allocation failed\n");
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
                fprintf(stderr, "enc_server: memory reallocation failed\n");
                exit(1);
            }
        }
        buffer[i++] = character;
    }
    buffer[i] = '\0';
    return buffer;
}

static int charToIndex(char symbol)
{
    return (symbol == ' ') ? 26 : symbol - 'A';
}

static char indexToChar(int value)
{
    return (value == 26) ? ' ' : 'A' + value;
}

static char *encryptText(const char *plainText, const char *keyText)
{
    size_t length = strlen(plainText);
    char *cipheredText = malloc(length + 1);
    if (!cipheredText)
    {
        fprintf(stderr, "enc_server: memory allocation failed\n");
        exit(1);
    }

    for (size_t pos = 0; pos < length; pos++)
    {
        int plainVal = charToIndex(plainText[pos]);
        int keyVal = charToIndex(keyText[pos]);
        int cipherVal = (plainVal + keyVal) % 27;
        cipheredText[pos] = indexToChar(cipherVal);
    }
    cipheredText[length] = '\0';
    return cipheredText;
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

int main(int argc, char *argv[])
{
    struct sockaddr_in serverAddress, clientAddress;
    if (argc < 2)
    {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0)
    {
        error("ERROR opening socket");
    }

    setupAddressStruct(&serverAddress, atoi(argv[1]));

    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        error("ERROR on binding");
    }

    listen(listenSocket, 5);

    while (1)
    {
        socklen_t sizeOfClientInfo = sizeof(clientAddress);
        int connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
        if (connectionSocket < 0)
        {
            error("ERROR on accept");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            error("ERROR on fork");
            close(connectionSocket);
            continue;
        }

        if (pid == 0)
        {
            close(listenSocket);

            char *clientID = readLine(connectionSocket);
            if (strcmp(clientID, "enc_client") != 0)
            {
                send(connectionSocket, "REJECT\n", strlen("REJECT\n"), 0);
                close(connectionSocket);
                exit(0);
            }
            sendAll(connectionSocket, "enc_server\n", strlen("enc_server\n"));

            char *plaintext = readLine(connectionSocket);
            char *key = readLine(connectionSocket);

            if (strlen(key) < strlen(plaintext))
            {
                fprintf(stderr, "enc_server: key too short\n");
                close(connectionSocket);
                exit(0);
            }

            char *ciphertext = encryptText(plaintext, key);
            sendAll(connectionSocket, ciphertext, strlen(ciphertext));
            sendAll(connectionSocket, "\n", 1);

            close(connectionSocket);
            exit(0);
        }

        close(connectionSocket);
    }

    close(listenSocket);
    return 0;
}
