#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define MYLIMIT 16384
#define MAX_LINES 100

int main(int argc, char *argv[])
{
    FILE *fp_in, *fp_out;
    char lines[MAX_LINES][MAX_LINES];
    char *line, err_msg[100];
    int count;
    size_t line_max;
    if (argc > 3)
    {
        strcpy(err_msg, "usage: reverse <input> <output>");
        fprintf(stderr, "%s\n", err_msg);
        exit(1);
    }

    // Handle Arguments
    if (argc == 1)
    {
        fp_in = stdin;
        fp_out = stdout;
    }
    else if (argc == 2)
    {
        fp_in = fopen(argv[1], "r");
        if (fp_in == NULL)
        {
            strcpy(err_msg, "reverse: cannot open file '");
            strcat(err_msg, argv[1]);
            strcat(err_msg, "'");
            fprintf(stderr, "%s\n", err_msg);
            exit(1);
        }
        fp_out = stdout;
    }
    else if (argc == 3)
    {
        // Handle input and output files are the same
        if (strcmp(argv[1], argv[2]) == 0)
        {
            strcpy(err_msg, "reverse: input and output file must differ");
            fprintf(stderr, "%s\n", err_msg);
            exit(1);
        }

        // Check for hard-linking
        int result1, result2;
        struct stat s1, s2;
        result1 = stat(argv[1], &s1);
        result2 = stat(argv[2], &s2);
        if ((result1 == 0) && (result2 == 0) && (s1.st_ino == s2.st_ino))
        {
            strcpy(err_msg, "reverse: input and output file must differ");
            fprintf(stderr, "%s\n", err_msg);
            exit(1);
        }

        // Input file
        fp_in = fopen(argv[1], "r");
        if (fp_in == NULL)
        {
            strcpy(err_msg, "reverse: cannot open file '");
            strcat(err_msg, argv[1]);
            strcat(err_msg, "'");
            fprintf(stderr, "%s\n", err_msg);
            exit(1);
        }

        // Output file
        fp_out = fopen(argv[2], "w");
        if (fp_out == NULL)
        {
            strcpy(err_msg, "reverse: cannot open file '");
            strcat(err_msg, argv[2]);
            strcat(err_msg, "'");
            fprintf(stderr, "%s\n", err_msg);
            exit(1);
        }
    }

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

    count = 0;

    // Read in lines
    while (getline(&line, &line_max, fp_in) > -1)
    {
        // Copy read line into the lines array
        strcpy(lines[count], line);
        line = malloc(line_max + 1);
        if (line == NULL)
        {
            // out of space
            exit(-1);
        }
        count++;
    }

    free(line);

    // Print lines in reverse
    while (count > 0)
    {
        fprintf(fp_out, "%s", lines[count - 1]);
        count--;
    }

    if (argc > 1)
        fclose(fp_in);
    if (argc > 2)
        fclose(fp_out);

    return 0;
}
