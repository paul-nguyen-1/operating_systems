// Name: Paul Nguyen
// Date: 5/25/2025
// Course: CS 374 - Operating Systems
// Programming Assignment 4: SMALLSH

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

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

int is_foreground_only = 0;

// Handler for SIGSTP to check foreground only mode
void handle_SIGTSTP(int signo)
{
    if (is_foreground_only == 0)
    {
        char *message = "\nEntering foreground-only mode (& is now ignored)\n: ";
        write(STDOUT_FILENO, message, strlen(message));
        is_foreground_only = 1;
    }
    else
    {
        char *message = "\nExiting foreground-only mode\n: ";
        write(STDOUT_FILENO, message, strlen(message));
        is_foreground_only = 0;
    }
    fflush(stdout);
}

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

    if (is_foreground_only)
    {
        curr_command->is_bg = false;
    }

    return curr_command;
}

int main()
{
    struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};
    
    SIGINT_action.sa_handler = SIG_IGN;
    sigfillset(&SIGINT_action.sa_mask);
    sigaction(SIGINT, &SIGINT_action, NULL);

    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    struct command_line *curr_command;
    int exit_status = 0;

    while (1)
    {
        // Check any completed background processes
        int child_status;
        pid_t done_pid;
        while ((done_pid = waitpid(-1, &child_status, WNOHANG)) > 0)
        {
            printf("background pid %d is done: ", done_pid);
            if (WIFEXITED(child_status))
            {
                printf("exit value %d\n", WEXITSTATUS(child_status));
            }
            else if (WIFSIGNALED(child_status))
            {
                printf("terminated by signal %d\n", WTERMSIG(child_status));
            }
            fflush(stdout);
        }

        curr_command = parse_input();
        if (curr_command == NULL)
        {
            continue;
        }

        // Built-in command for exit
        if (strcmp(curr_command->argv[0], "exit") == 0)
        {
            exit(0);
        }

        // Built-in command for cd
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
            // No path provided
            else
            {
                chdir(getenv("HOME"));
            }
        }

        // Built-in command for status
        else if (strcmp(curr_command->argv[0], "status") == 0)
        {
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

        else
        {
            pid_t spawnpid = fork();

            switch (spawnpid)
            {
            case -1:
                perror("fork() failed!");
                exit(1);
                break;

            case 0:
                if (!curr_command->is_bg)
                {
                    SIGINT_action.sa_handler = SIG_DFL;
                }
                else
                {
                    SIGINT_action.sa_handler = SIG_IGN;
                }
                sigaction(SIGINT, &SIGINT_action, NULL);
                SIGTSTP_action.sa_handler = SIG_IGN;
                sigaction(SIGTSTP, &SIGTSTP_action, NULL);

                // handle input redirection
                if (curr_command->input_file != NULL)
                {
                    int input_fd = open(curr_command->input_file, O_RDONLY);
                    if (input_fd == -1)
                    {
                        fprintf(stderr, "cannot open %s for input\n", curr_command->input_file);
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

            default:
                if (curr_command->is_bg)
                {
                    printf("background pid is %d\n", spawnpid);
                    fflush(stdout);
                }
                else
                {
                    waitpid(spawnpid, &exit_status, 0);
                    if (WIFSIGNALED(exit_status))
                    {
                        printf("terminated by signal %d\n", WTERMSIG(exit_status));
                        fflush(stdout);
                    }
                }
                break;
            }
        }
    }

    return 0;
}
