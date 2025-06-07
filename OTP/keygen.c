#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Please make sure you only have one key length and that it is positive \n");
        return 1;
    }

    int length = atoi(argv[1]);
    if (length <= 0)
    {
        fprintf(stderr, "Key length must be positive \n");
        return 1;
    }

    const char allowed[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    srand(time(NULL));

    // Generate a random key of the specified length
    for (int i = 0; i < length; i++)
    {
        printf("%c", allowed[rand() % 27]);
    }

    printf("\n");
    return 0;
}
