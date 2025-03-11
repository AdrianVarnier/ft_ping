#include "ft_ping.h"

int    send_ping(t_data* data)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    if (sendto(data->sockfd, &data->packet, sizeof(data->packet), 0, data->res->ai_addr, data->res->ai_addrlen) < 0)
        return -1;
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    memset(&data->response, 0, sizeof(data->response));
    if ((data->bytes_received = recvfrom(data->sockfd, data->response, PACKET_SIZE + IP_HEADER_SIZE, 0, (struct sockaddr*)&sender_addr, &sender_len)) <= 0)
        return -1;
    gettimeofday(&end, NULL);
    data->ttl = ((struct ip *)data->response)->ip_ttl;
    data->rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    return 0;
}

void    handle_response(t_data* data, int ret)
{
    struct icmphdr *response = (struct icmphdr *)(data->response + (((struct ip *)data->response)->ip_hl * 4));
    if (response->type == ICMP_ECHOREPLY && ret != -1)
    {
        update_rtt(data);
        if (!data->q)
            display_ping_info(data);
    }
    else
    {
        data->errors++;
        if (data->v && !data->q)
        {
           if (response->type == ICMP_DEST_UNREACH)
                printf("icmp_seq=%d Host Unreachable\n", data->seq);
            if (response->type == ICMP_TIME_EXCEEDED)
                printf("icmp_seq=%d Time to live exceeded\n", data->seq);
            if (response->type == ICMP_REDIRECT)
                printf("icmp_seq=%d Redirect Host\n", data->seq);
            if (response->type == ICMP_PARAMETERPROB)
                printf("icmp_seq=%d Parameter problem\n", data->seq);
        }
    }
}

void    update_rtt(t_data* data)
{
    data->rtt_count++;
    data->rtt_sum += data->rtt;
    data->rtt_sqr_sum += pow(data->rtt, 2);
    if (data->rtt_min > data->rtt || data->rtt_min == 0)
        data->rtt_min = data->rtt;
    if (data->rtt_max < data->rtt)
        data->rtt_max = data->rtt;   
}