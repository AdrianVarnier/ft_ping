#include "ft_ping.h"

void    set_addr_hint(t_data* data)
{
    data->hints.ai_family = AF_INET;
    data->hints.ai_socktype = SOCK_RAW;
    data->hints.ai_protocol = IPPROTO_ICMP;
}

int     resolve_addr(t_data* data, char* addr)
{
    if (getaddrinfo(addr, NULL, &data->hints, &data->res) != 0)
    {
        fprintf(stderr, "ft_ping: unknow host\n");
        exit_clean(1);
    }
    return 0;
}

