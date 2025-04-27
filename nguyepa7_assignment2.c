// Name: Paul Nguyen
// Date: 04/27/2025
// Course: CS 374 - Operating Systems
// Programming Assignment 2

#include <stdio.h>
#include <stdlib.h>

// Source code from sample file in canvas
struct movie
{
    char *title;
    int year;
    char **languages;
    int rating;
    struct movie *next;
};

struct movie *processMovieFile(char *filePath)
{
    char *currLine = NULL;
    size_t len = 0;
    struct movie *head = NULL; // Initialize the head of the linked list

    // Open the specified file for reading only
    FILE *movieFile = fopen(filePath, "r");

    // Read the file line by line
    while (getline(&currLine, &len, movieFile) != -1)
    {
        printf("%s", currLine);
    }

    // Allocate memory for a new movie node
    struct movie *newMovie = malloc(sizeof(struct movie));

    // Add the new movie to the linked list
    newMovie->next = head;
    head = newMovie;

    // Free the memory allocated by getline for currLine
    free(currLine);
    // Close the file
    fclose(movieFile);
    return head; // Return the head of the linked list
}

struct movie
{
    char *title;
    int year;
    char **languages;
    int rating;
    struct movie *next;
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("You must provide the name of the file to process\n");
        printf("Example usage: ./movies movies.csv\n");
        return EXIT_FAILURE;
    }
    struct movie *movieData = processMovieFile(argv[1]);

    {
        int choice;
        printf("Enter a choice from 1 to 4: \n");

        printf("1. Show movies released in the specified year \n");
        printf("2. Show highest rated movie for each year \n");
        printf("3. Show the title and year of release of all movies in a specific language \n");
        printf("4. Exit from the program \n");
        scanf("%d", &choice);

        printf("You entered an incorrect choice. Try again \n");
        return 0;
    }
};
