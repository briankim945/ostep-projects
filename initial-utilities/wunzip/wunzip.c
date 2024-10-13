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
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }
    else
    {
        // Handling file name arguments
        unsigned char buf[4];
        char c;
        int arg_i, c_count;

        arg_i = 1;
        while (arg_i < argc)
        {
            // Open file
            FILE *fp = fopen(argv[arg_i], "r");
            if (fp == NULL)
            {
                printf("wunzip: cannot open file\n");
                exit(1);
            }

            while ((int)fread(buf, 4, 1, fp) > 0)
            {
                c_count = buf[0] + (buf[1] * 256) + (buf[2] * 256 * 256) + (buf[3] * 256 * 256 * 256);
                fread(buf, 1, 1, fp);
                c = buf[0];
                for (int i = 0; i < c_count; i++)
                {
                    printf("%c", c);
                }
            }

            // close file stream
            fclose(fp);

            arg_i++;
        }

        return 0;
    }
}
