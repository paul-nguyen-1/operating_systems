// Name: Paul Nguyen
// Date: 04/27/2025
// Course: CS 374 - Operating Systems
// Programming Assignment 2: Movies

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

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
            else if (choice == 3) // find file name
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

// code from assignment 2
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
    int choice;
    char filePath[256];

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
