#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MYLIMIT 16384

void handle_input(FILE *fp, char *target);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("wgrep: searchterm [file ...]\n");
        return 1;
    }
    else if (argc == 2)
    {
        // Handling STDIN inputs when given only two arguments

        handle_input(stdin, argv[1]); // Call search of input

        // close file stream
        return 0;
    }
    else
    {
        // Handling file name arguments
        int arg_i;

        arg_i = 2;
        while (arg_i < argc)
        {
            // Open file
            FILE *fp = fopen(argv[arg_i], "r");
            if (fp == NULL)
            {
                printf("wgrep: cannot open file\n");
                exit(1);
            }

            handle_input(fp, argv[1]); // Call search of file

            // close file stream
            fclose(fp);

            arg_i++;
        }

        return 0;
    }
}

void handle_input(FILE *fp, char *target)
{
    size_t target_len = strlen(target);
    char *line;
    int line_max, line_len, line_i, target_i;
    bool cont;

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

    // Read lines
    // line_max + 1 leaves room for the null byte added by fgets().
    line = malloc(line_max + 1);
    if (line == NULL)
    {
        // out of space
        exit(-1);
    }

    while (fgets(line, line_max + 1, fp) != NULL)
    {
        // Verify that a full line has been read ...
        // If not, report an error or prepare to treat the
        // next time through the loop as a read of a
        // continuation of the current line.

        // Process line ...
        line_i = 0;
        cont = true;
        line_len = strlen(line);
        while ((target_len <= line_len) && (line_i < (line_len - target_len)) && cont)
        {
            // Run through line to see if line matches target content
            target_i = 0;
            while ((target_i < target_len) && ((line_i + target_i) < line_len) && (line[line_i + target_i] == target[target_i]))
            {
                target_i++;
            }
            if (target_i >= target_len)
            {
                printf("%s", line); // Print current line
                cont = false;
            }
            line_i++;
        }
    }
    free(line);
}
