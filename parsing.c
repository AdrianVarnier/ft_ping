#include "ft_ping.h"

static void    invalid_flag(char c)
{
    fprintf(stderr, "invalid option '%c'\n", c);
    display_help();
    exit_clean(1);
}

static int    get_flag(char* s, t_data* data)
{
    if (strlen(s) < 2 || s[0] != '-')
        invalid_flag(0);
    if (s[1] == '?')
    {
        display_help();
        exit_clean(0);
    }
    else if (s[1] == 'v')
        data->v = 1;
    else if (s[1] == 'q')
        data->q = 1;    
    else if (s[1] == 't')
        data->t = 1;
    else
        invalid_flag(s[1]);
    return (s[1]);
}

void    parse(int argc, char** argv, t_data* data)
{
    for (int i = 1; i < argc - 1; i++)
    {
        int flag = get_flag(argv[i], data);
        if (flag == 't' && (i + 1 < argc - 1))
            data->ttl_max = atoi(argv[++i]);
    }
}