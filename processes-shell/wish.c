#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MYLIMIT 16384

int main(int argc, char *argv[])
{
    char *line, *dir_chunk, *arg_chunk, *word, *command;
    size_t line_max;
    FILE *fp;
    bool redirect[100], exited = false;
    char *paths[100], command_list[100][100], arg_list[100][100][100], *files[100];
    char error_message[30] = "An error has occurred\n";
    int arg_counts[100], rc_list[100], output_count[100];
    int command_index, path_count, i, j, builtin_index, redirect_count /*, status*/;
    const char *delimiters = " \t\n", *arg_delimiters = "&", *dir_delimiters = ">";
    pid_t wpid;

    // Handle line configurations
    if (LINE_MAX >= MYLIMIT)
    {
        // Use maximum line size of MYLIMIT. If LINE_MAX is
        // bigger than our limit, sysconf() cannot report a
        // smaller limit.
        line_max = MYLIMIT;
    }
    else
    {
        long limit = sysconf(_SC_LINE_MAX);
        line_max = (limit < 0 || limit > MYLIMIT) ? MYLIMIT : (int)limit;
    }

    if (argc == 1)
    {
        fp = stdin;
    }
    else if (argc == 2)
    {
        fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
    }
    else
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
    }

    // Read lines
    // line_max + 1 leaves room for the null byte added by fgets().
    line = malloc(line_max + 1);
    if (line == NULL)
    {
        // out of space
        exit(-1);
    }

    arg_chunk = malloc(line_max + 1);
    if (arg_chunk == NULL)
    {
        // out of space
        exit(-1);
    }

    // set /bin as default starting value of paths
    paths[0] = strdup("/bin");
    path_count = 1;

    if (fp == stdin)
        printf("wish> ");

    // Read in lines
    while (!exited && (getline(&line, &line_max, fp) > -1))
    {
        builtin_index = -1;
        // arg_index = 0;
        command_index = 0;

        // break line into multiple commands
        while ((arg_chunk = strsep(&line, arg_delimiters)) != NULL)
        {
            if (*arg_chunk != '\0')
            {
                command = NULL;
                arg_counts[command_index] = 0;
                redirect[command_index] = false;
                files[command_index] = NULL;
                output_count[command_index] = 0;
                redirect_count = 0;
                // handle breaks of '>'
                while ((dir_chunk = strsep(&arg_chunk, dir_delimiters)) != NULL)
                {
                    if (*dir_chunk != '\0')
                    {
                        if (redirect_count > 0)
                            redirect[command_index - 1] = true;
                        // break line into words
                        while ((word = strsep(&dir_chunk, delimiters)) != NULL)
                        {
                            if (*word != '\0')
                            {
                                if (command == NULL)
                                {
                                    command = word;
                                    // check for built-in commands
                                    if ((strcmp(command, "exit") == 0) || (strcmp(command, "path") == 0) || (strcmp(command, "cd") == 0))
                                        builtin_index = command_index;
                                    // copying into command lists
                                    strcpy(command_list[command_index], command);
                                    command_index++;
                                    // copying into first argument
                                    strcpy(arg_list[command_index - 1][0], command);
                                    arg_counts[command_index - 1] = 1;
                                    // setting last element in arguments (so far) to null
                                    memset(arg_list[command_index - 1][arg_counts[command_index - 1]], 0, 100);
                                }
                                else if (redirect[command_index - 1]) // check if redirect
                                {
                                    if (files[command_index - 1] == NULL) // if no redirect file given yet
                                    {
                                        files[command_index - 1] = strdup(word);
                                    }
                                    output_count[command_index - 1]++; // increase count of output files
                                }
                                else
                                {
                                    strcpy(arg_list[command_index - 1][arg_counts[command_index - 1]], word);
                                    arg_counts[command_index - 1]++;
                                    memset(arg_list[command_index - 1][arg_counts[command_index - 1]], 0, 100);
                                }
                            }
                        }
                        redirect_count++;
                    }
                }
            }
        }

        if (builtin_index > -1)
        {
            // handle commands
            if (strcmp(command_list[builtin_index], "exit") == 0) // handle exit
            {
                if (arg_counts[builtin_index] == 1) // only if no arguments
                {
                    exited = true;
                }
                else
                {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }
            else if (strcmp(command_list[builtin_index], "path") == 0) // handle path
            {
                if (arg_counts[builtin_index] == 1) // handle erasing paths
                {
                    path_count = 0;
                }
                else // handle adding paths
                {
                    for (i = 1; i < arg_counts[builtin_index]; i++)
                    {
                        paths[path_count] = strndup(arg_list[builtin_index][i], strlen(arg_list[builtin_index][i]));
                        path_count++;
                    }
                }
            }
            else if (strcmp(command_list[builtin_index], "cd") == 0) // handle cd
            {
                if (arg_counts[builtin_index] == 2) // run cd only if there is one arg
                {
                    if (chdir((const char *)arg_list[builtin_index][1]) != 0)
                    {
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                }
                else
                {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }
        }
        else // handle any non-hardcoded commands
        {
            // check that arguments and redirects are valid
            for (j = 0; j < command_index; j++)
            {
                for (i = 0; i < path_count; i++) // search saved paths
                {
                    if (strcmp(paths[i], command_list[j]) == 0) // check if directly exists
                        break;

                    char *tmp_word = strdup(paths[i]); // check if within directory
                    strcat(tmp_word, "/");
                    strcat(tmp_word, command_list[j]);

                    if (access(tmp_word, X_OK) > -1)
                    {
                        strcpy(command_list[j], strdup(tmp_word));
                        break;
                    }
                }

                if (i >= path_count) // cannot find command in paths
                {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    break;
                }

                if (redirect[j] && (output_count[j] != 1)) // redirect without valid output file
                {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    break;
                }
            }

            if (j >= command_index) // check that all paths exist
            {
                // status = 0;

                for (i = 0; i < command_index; i++)
                {
                    rc_list[i] = fork();

                    if (rc_list[i] < 0)
                    {
                        // fork failed; exit
                        fprintf(stderr, "fork failed\n");
                        exit(1);
                    }
                    else if (rc_list[i] == 0)
                    {
                        // child (new process)
                        // now exec command

                        char *cur_args[arg_counts[i]];
                        char *command;
                        j = 0;
                        while (arg_list[i][j][0] != '\0')
                        {
                            cur_args[j] = strndup(arg_list[i][j], (int)strlen(arg_list[i][j]));
                            j++;
                        }
                        cur_args[j] = NULL;

                        command = strdup(command_list[i]);

                        // handle redirect
                        if (redirect[i] && (output_count[i] == 1))
                        {
                            int fd = open(files[i], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

                            dup2(fd, 1); // redirect stdout to file
                            dup2(fd, 2); // redirect stderr to file

                            close(fd); // close fd as no longer needed, dups are sufficient
                        }

                        execv(command, cur_args);
                        exit(1);
                    }
                }

                for (i = 0; i < command_index; i++)
                {
                    if (rc_list[i] > 0)
                    {
                        while ((wpid = wait(NULL)) > 0)
                            ;
                    }
                }
            }
        }
        if ((fp == stdin) && !exited)
            printf("wish> ");
    }

    free(line);
    free(arg_chunk);

    return 0;
}
