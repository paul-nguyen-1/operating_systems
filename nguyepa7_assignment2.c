// Name: Paul Nguyen
// Date: 04/27/2025
// Course: CS 374 - Operating Systems
// Programming Assignment 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct movie
{
    char *title;
    int year;
    char **languages;
    int rating;
    struct movie *next;
};

// Source code used from sample file in canvas
struct movie *processMovieFile(char *filePath)
{
    char *currLine = NULL;
    size_t len = 0;
    struct movie *head = NULL;
    struct movie *tail = NULL;

    FILE *movieFile = fopen(filePath, "r");

    // Skip header
    getline(&currLine, &len, movieFile);

    // Read the file line by line
    while (getline(&currLine, &len, movieFile) != -1)
    {
        // Allocate memory for new movie node
        struct movie *newMovie = malloc(sizeof(struct movie));

        // Parse and store movie data
        char *token = strtok(currLine, ",");
        newMovie->title = strdup(token);

        token = strtok(NULL, ",");
        newMovie->year = atoi(token);

        token = strtok(NULL, ",");
        newMovie->languages = NULL;

        token = strtok(NULL, ",");
        newMovie->rating = atoi(token);

        newMovie->next = NULL;

        // Add movie to the linked list
        if (head == NULL)
        {
            head = newMovie;
            tail = newMovie;
        }
        else
        {
            tail->next = newMovie;
            tail = newMovie;
        }
    }

    // Free the memory allocated by getline for currLine
    free(currLine);
    // Close the file
    fclose(movieFile);
    return head;
}

void showMoviesByYear(struct movie *list, int year)
{
    int currentYear = 0;
    struct movie *curr = list;
    while (curr != NULL)
    {
        if (curr->year == year)
        {
            printf("%s\n", curr->title);
            currentYear = 1;
        }
        curr = curr->next;
    }

    if (!currentYear)
    {
        printf("No data about movies released in the year %d\n", year);
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("You must provide the name of the file to process\n");
        printf("Example usage: ./movies movies.csv\n");
        return EXIT_FAILURE;
    }
    struct movie *movieData = processMovieFile(argv[1]);

    int choice;
    printf("Enter a choice from 1 to 4: \n");

    printf("1. Show movies released in the specified year \n");
    printf("2. Show highest rated movie for each year \n");
    printf("3. Show the title and year of release of all movies in a specific language \n");
    printf("4. Exit from the program \n");
    scanf("%d", &choice);

    if (choice == 1)
    {
        int year;
        printf("Enter the year for which you want to see movies: ");
        scanf("%d", &year);
        showMoviesByYear(movieData, year);
    }

    printf("You entered an incorrect choice. Try again \n");
    return 0;
}
