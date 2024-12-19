#include "ft_ping.h"

static void    print_info()
{
    fprintf(stdout, "\nUsage\n");
    fprintf(stdout, "\tft_ping [options] <destination>\n\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "\t-v: verbose output\n");
    fprintf(stdout, "\t-?: display help message\n\n");
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "ft_ping: usage error: Destination address required");
        return (1);
    }
    int verbose = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-v") == 0)
            verbose = 1;
        else if (strcmp(argv[i], "-?") == 0)
        {
            print_info();
            return (0);
        }
        else
        {
            while (1)
                ft_ping(argv[i], verbose);
        }
    }
    return (0);
}