#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512

struct command_line
{
    char *argv[MAX_ARGS + 1];
    int argc;
    char *input_file;
    char *output_file;
    bool is_bg;
};

struct command_line *parse_input()
{
    char input[INPUT_LENGTH];
    struct command_line *curr_command = (struct command_line *)calloc(1, sizeof(struct command_line));

    // Get input
    printf(": ");
    fflush(stdout);
    fgets(input, INPUT_LENGTH, stdin);

    // Ignore blank lines or comment lines
    if (input[0] == '\n' || input[0] == '#')
    {
        return NULL;
    }

    // Tokenize the input
    char *token = strtok(input, " \n");
    while (token)
    {
        if (!strcmp(token, "<"))
        {
            curr_command->input_file = strdup(strtok(NULL, " \n"));
        }
        else if (!strcmp(token, ">"))
        {
            curr_command->output_file = strdup(strtok(NULL, " \n"));
        }
        else if (!strcmp(token, "&"))
        {
            curr_command->is_bg = true;
        }
        else
        {
            curr_command->argv[curr_command->argc++] = strdup(token);
        }
        token = strtok(NULL, " \n");
    }

    // Check if last argument is "&"
    if (curr_command->argc > 0 && strcmp(curr_command->argv[curr_command->argc - 1], "&") == 0)
    {
        curr_command->is_bg = true;
    }
    return curr_command;
}

int main()
{
    struct command_line *curr_command;
    int exit_status = 0; // track exit status of last foreground process

    while (true)
    {
        curr_command = parse_input();
        // blank line or comment
        if (curr_command == NULL)
        {
            continue;
        }

        // Built-in command: exit
        if (strcmp(curr_command->argv[0], "exit") == 0)
        {
            // In a future version: kill background processes before exiting
            exit(0);
        }

        // Built-in command: cd
        else if (strcmp(curr_command->argv[0], "cd") == 0)
        {
            // If path is provided
            if (curr_command->argc > 1)
            {
                if (chdir(curr_command->argv[1]) != 0)
                {
                    perror("cd");
                }
            }
            // No path provided — go to $HOME
            else
            {
                chdir(getenv("HOME"));
            }
        }

        // Built-in command: status
        else if (strcmp(curr_command->argv[0], "status") == 0)
        {
            // Display last foreground process's status
            if (WIFEXITED(exit_status))
            {
                printf("exit value %d\n", WEXITSTATUS(exit_status));
            }
            else if (WIFSIGNALED(exit_status))
            {
                printf("terminated by signal %d\n", WTERMSIG(exit_status));
            }
            fflush(stdout);
        }

        // Non-built-in commands
        else
        {
            // modified code snippet from canvas
            pid_t spawnpid = fork();

            switch (spawnpid)
            {
            case -1:
                perror("fork() failed!");
                exit(1);
                break;

            case 0: // child
                if (curr_command->input_file != NULL)
                {
                    int input_fd = open(curr_command->input_file, O_RDONLY);
                    if (input_fd == -1)
                    {
                        perror("cannot open input file");
                        exit(1);
                    }
                    dup2(input_fd, 0);
                    close(input_fd);
                }

                // handle output redirection
                if (curr_command->output_file != NULL)
                {
                    int output_fd = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (output_fd == -1)
                    {
                        perror("cannot open output file");
                        exit(1);
                    }
                    dup2(output_fd, 1);
                    close(output_fd);
                }

                execvp(curr_command->argv[0], curr_command->argv);
                // if execvp returns, it failed
                perror(curr_command->argv[0]);
                exit(1);
                break;

            default: // parent
                int child_status;
                waitpid(spawnpid, &child_status, 0);
                exit_status = child_status;
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}
