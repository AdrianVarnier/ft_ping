#ifndef FT_PING_H
#define FT_PING_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>

#define ICMP_HEADER_SIZE 8
#define PAYLOAD_SIZE 56
#define PACKET_SIZE PAYLOAD_SIZE + ICMP_HEADER_SIZE
#define IP_HEADER_SIZE 20
#define TIMEOUT_S 1
#define TIMEOUT_MS 0
#define TTL_MAX 1

typedef struct s_icmp_packet
{
    struct icmphdr  header;
    char            payload[PAYLOAD_SIZE];
} t_icmp_packet;

typedef struct s_data
{
    // request parameters
    t_icmp_packet   packet;
    char            response[PACKET_SIZE + IP_HEADER_SIZE];
    char            ip[INET_ADDRSTRLEN];
    struct addrinfo *res;
    struct addrinfo hints;
    int             sockfd;
    int             v;
    struct timeval  timeout;
    int             ttl_max;

    // statistics
    int             seq;
    int             ttl;
    double          rtt;
    int             rtt_count;
    double          rtt_min;
    double          rtt_max;
    double          rtt_sum;
    double          rtt_sqr_sum;
    ssize_t         bytes_received;
    int             errors;
}              t_data;

#endif