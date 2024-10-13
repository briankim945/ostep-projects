#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MYLIMIT 16384

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }
    else
    {
        // Handling file name arguments
        char *line, c;
        int arg_i, line_max, line_len, line_i, c_count;
        bool unset = true;

        arg_i = 1;
        while (arg_i < argc)
        {
            // Open file
            FILE *fp = fopen(argv[arg_i], "r");
            if (fp == NULL)
            {
                printf("wzip: cannot open file\n");
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
                exit(-1);
            }

            while (fgets(line, line_max + 1, fp) != NULL)
            {
                // Verify that a full line has been read ...
                // If not, report an error or prepare to treat the
                // next time through the loop as a read of a
                // continuation of the current line.

                // Process line ...
                line_len = strlen(line);

                // Get first char
                if ((line_len > 0) && unset)
                {
                    c = line[0];
                    c_count = 1;
                    unset = false;
                    line_i = 1;
                }
                else
                {
                    line_i = 0;
                }

                // Run through line
                while (line_i < line_len)
                {
                    // Run through line to see if line matches target content
                    if (line[line_i] == c)
                    {
                        c_count++;
                    }
                    else
                    {
                        if (fwrite(&c_count, 4, 1, stdout) < 0)
                        {
                            exit(2);
                        }
                        printf("%c", c);
                        c = line[line_i];
                        c_count = 1;
                    }
                    line_i++;
                }
            }
            free(line);

            // close file stream
            fclose(fp);

            arg_i++;
        }

        if (!unset)
        {
            fwrite(&c_count, 4, 1, stdout);
            printf("%c", c);
        }

        return 0;
    }
}
