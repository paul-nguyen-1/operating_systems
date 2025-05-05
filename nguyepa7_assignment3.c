// Name: Paul Nguyen
// Date: 5/4/2025
// Course: CS 374 - Operating Systems
// Programming Assignment 3: FileSearch

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

struct movie
{
    char *title;
    int year;
    struct movie *next;
};

// Source code used from sample file in canvas Exploration: Directories
int getFile(char *filePath, int choice) // 1 = largest, 2 = smallest, 3 = name
{
    DIR *currDir;
    struct dirent *entry;
    struct stat dirStat;
    float bytes = 0.0;
    int fileFound = 0;

    // Open the current directory
    currDir = opendir(".");

    // Go through all the entries
    while ((entry = readdir(currDir)) != NULL)
    {
        // Get meta-data for the current entry
        if (stat(entry->d_name, &dirStat) == -1)
            continue;

        if (S_ISREG(dirStat.st_mode))
        {
            if ((choice == 1 || choice == 2) &&
                strstr(entry->d_name, "movies_") == entry->d_name &&
                strstr(entry->d_name, ".csv"))
            {
                if (choice == 1) // largest
                {
                    if (dirStat.st_size > bytes)
                    {
                        bytes = dirStat.st_size;
                        strcpy(filePath, entry->d_name);
                        fileFound = 1;
                    }
                }
                else if (choice == 2) // smallest
                {
                    if (bytes == 0.0 || dirStat.st_size < bytes)
                    {
                        bytes = dirStat.st_size;
                        strcpy(filePath, entry->d_name);
                        fileFound = 1;
                    }
                }
            }
            else if (choice == 3) // file name
            {
                if (strcmp(entry->d_name, filePath) == 0)
                {
                    fileFound = 1;
                    break;
                }
            }
        }
    }

    // Close the directory
    closedir(currDir);
    return fileFound;
}

// Source code used from assignment 2
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
        token = strtok(NULL, ",");

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

void processFile(char *filePath)
{
    struct movie *movies = processMovieFile(filePath);
    int randomNumber = rand();
    char directory[256];
    snprintf(directory, sizeof(directory), "nguyepa7.movies.%d", randomNumber);

    if (mkdir(directory, 0750) != 0)
    {
        return;
    }

    printf("Created directory with name %s\n", directory);

    struct movie *curr = movies;
    while (curr != NULL)
    {
        char fileName[256];
        snprintf(fileName, sizeof(fileName), "%s/%d.txt", directory, curr->year);

        FILE *fp = fopen(fileName, "a");
        if (fp == NULL)
        {
            curr = curr->next;
            continue;
        }

        fprintf(fp, "%s\n", curr->title);
        fclose(fp);
        chmod(fileName, 0640);

        curr = curr->next;
    }

    while (movies != NULL)
    {
        struct movie *temp = movies;
        movies = movies->next;
        free(temp->title);
        free(temp);
    }
}

int main(int argc, char **argv)
{
    int choice;
    char filePath[256];
    srand(time(NULL));

    while (true)
    {
        printf("1. Select file to process \n");
        printf("2. Exit the program \n");
        printf("Enter a choice 1 or 2: ");
        scanf("%d", &choice);

        if (choice == 1)
        {
            int fileChoice;
            while (1)
            {
                printf("\nWhich file do you want to process?\n");
                printf("Enter 1 to pick the largest file\n");
                printf("Enter 2 to pick the smallest file\n");
                printf("Enter 3 to specify the name of a file\n");
                printf("Enter a choice from 1 to 3: ");
                scanf("%d", &fileChoice);

                if (fileChoice == 1 || fileChoice == 2)
                {
                    if (getFile(filePath, fileChoice))
                    {
                        printf("Now processing the chosen file named %s\n", filePath);
                        processFile(filePath);
                        break;
                    }
                    else
                    {
                        printf("No suitable file found. Please try again.\n");
                    }
                }
                else if (fileChoice == 3)
                {
                    printf("Enter the complete filename: ");
                    scanf("%s", filePath);

                    if (getFile(filePath, 3))
                    {
                        printf("Now processing the chosen file named %s\n", filePath);
                        processFile(filePath);
                        break;
                    }
                    else
                    {
                        printf("The file %s was not found. Try again.\n", filePath);
                    }
                }
                else
                {
                    printf("Invalid choice. Please enter 1, 2, or 3.\n");
                }
            }
        }
        else if (choice == 2)
        {
            printf("2. Exit the program \n");
            break;
        }
        else
        {
            printf("You entered an incorrect choice. Try again.\n");
        }
    }

    return EXIT_SUCCESS;
}
