#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define MYLIMIT 16384

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return 0;
    }
    else
    {
        int arg_i;

        arg_i = 1;
        while (arg_i < argc)
        {
            char *line;
            int line_max;

            // Open file
            FILE *fp = fopen(argv[arg_i], "r");
            if (fp == NULL)
            {
                printf("wcat: cannot open file\n");
                exit(1);
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
                return -1;
            }

            while (fgets(line, line_max + 1, fp) != NULL)
            {
                // Verify that a full line has been read ...
                // If not, report an error or prepare to treat the
                // next time through the loop as a read of a
                // continuation of the current line.

                // Process line ...
                printf("%s", line); // Print current line
            }
            free(line);

            // close file stream
            fclose(fp);

            arg_i++;
        }

        return 0;
    }
}
