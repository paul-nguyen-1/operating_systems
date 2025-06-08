#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

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
        fprintf(stderr, "dec_server: memory allocation failed\n");
        exit(1);
    }

    size_t i = 0;
    char character;
    while (recv(sockFD, &character, 1, 0) == 1)
    {
        if (character == '\n')
            break;

        if (i >= bufSize - 1)
        {
            bufSize *= 2;
            buffer = realloc(buffer, bufSize);
            if (!buffer)
            {
                fprintf(stderr, "dec_server: memory reallocation failed\n");
                exit(1);
            }
        }
        buffer[i++] = character;
    }
    buffer[i] = '\0';
    return buffer;
}

static int getSymbolValue(char symbol)
{
    return (symbol == ' ') ? 26 : symbol - 'A';
}

static char getSymbolFromValue(int value)
{
    return (value == 26) ? ' ' : 'A' + value;
}

static char *decryptText(const char *cipherText, const char *keyText)
{
    size_t length = strlen(cipherText);
    char *plainText = malloc(length + 1);
    if (!plainText)
    {
        fprintf(stderr, "dec_server: memory allocation failed\n");
        exit(1);
    }

    for (size_t pos = 0; pos < length; pos++)
    {
        int cipherVal = getSymbolValue(cipherText[pos]);
        int keyVal = getSymbolValue(keyText[pos]);
        int plainVal = (cipherVal - keyVal + 27) % 27;
        plainText[pos] = getSymbolFromValue(plainVal);
    }
    plainText[length] = '\0';
    return plainText;
}

static void sendAll(int fd, const char *buffer, size_t length)
{
    while (length)
    {
        ssize_t sent = send(fd, buffer, length, 0);
        if (sent < 0)
        {
            perror("send");
            exit(1);
        }
        if (sent == 0)
        {
            fprintf(stderr, "sendAll connection closed\n");
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
        error("ERROR opening socket");

    setupAddressStruct(&serverAddress, atoi(argv[1]));

    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        error("ERROR on binding");

    listen(listenSocket, 5);

    while (1)
    {
        socklen_t sizeOfClientInfo = sizeof(clientAddress);
        int socketFD = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
        if (socketFD < 0)
        {
            error("ERROR on accept");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            error("ERROR on fork");
            close(socketFD);
            continue;
        }

        if (pid == 0)
        {
            close(listenSocket);

            char *clientID = readLine(socketFD);
            if (strcmp(clientID, "dec_client") != 0)
            {
                send(socketFD, "REJECT\n", strlen("REJECT\n"), 0);
                close(socketFD);
                exit(0);
            }
            send(socketFD, "dec_server\n", strlen("dec_server\n"), 0);

            char *ciphertext = readLine(socketFD);
            char *key = readLine(socketFD);

            if (strlen(key) < strlen(ciphertext))
            {
                fprintf(stderr, "dec_server: key too short\n");
                close(socketFD);
                exit(0);
            }

            char *plaintext = decryptText(ciphertext, key);
            sendAll(socketFD, plaintext, strlen(plaintext));
            sendAll(socketFD, "\n", 1);

            close(socketFD);
            exit(0);
        }

        close(socketFD);
    }

    close(listenSocket);
    return 0;
}
