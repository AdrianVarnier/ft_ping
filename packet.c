#include "ft_ping.h"

uint16_t checksum(void *ptr, int len)
{
    uint16_t* data = ptr;
    uint32_t sum = 0;
    for (; len > 1; len -= 2)
        sum += *data++;
    if (len == 1)
        sum += *(uint8_t*)data;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

void    set_echo_header(t_data* data)
{
    data->packet.header.type = ICMP_ECHO;
    data->packet.header.code = 0;
    data->packet.header.un.echo.id = getpid() & 0xFFFF;
    data->packet.header.un.echo.sequence = data->seq;
    data->packet.header.checksum = checksum(&data->packet, sizeof(data->packet));
}

void    update_packet(t_data *data)
{
    data->packet.header.un.echo.sequence = ++data->seq;
    data->packet.header.checksum = 0;
    data->packet.header.checksum = checksum(&data->packet, sizeof(data->packet));
}