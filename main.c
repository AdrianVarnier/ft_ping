#include "ft_ping.h"

t_data data;

static void    init_data(t_data* data)
{
    memset(data, 0, sizeof(t_data));
    set_addr_hint(data);
    set_echo_header(data);

}

static void    init_socket(t_data* data)
{
    if ((data->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        exit(1);
    setsockopt(data->sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    data->timeout.tv_sec = TIMEOUT_S;
    data->timeout.tv_usec = TIMEOUT_MS;
    setsockopt(data->sockfd, SOL_SOCKET, SO_RCVTIMEO, &data->timeout, sizeof(data->timeout));
    if (!data->t)
        data->ttl_max = TTL_MAX;
    setsockopt(data->sockfd, IPPROTO_IP, IP_TTL, &data->ttl_max, sizeof(data->ttl_max));
}

int     main(int argc, char** argv)
{
    int ret = 0;

    signal(SIGINT, handle_sigint);
    init_data(&data);
    parse(argc, argv, &data);
    init_socket(&data);
    resolve_addr(&data, argv[argc - 1]);
    display_dest_info(&data, argv[argc - 1]);
    while (1)
    {
        ret = send_ping(&data);
        handle_response(&data, ret);
        update_packet(&data);
        sleep(1);
    }
    return 0;
}