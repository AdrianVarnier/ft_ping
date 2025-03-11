#include "ft_ping.h"

extern t_data data;

void    free_data(t_data *data)
{
    close(data->sockfd);
    freeaddrinfo(data->res);  
}

void    exit_clean(int n)
{
    free_data(&data);
    exit(n);
}

void handle_sigint(int sig)
{
    (void)sig;
    display_stats(&data);
    exit_clean(0);
}