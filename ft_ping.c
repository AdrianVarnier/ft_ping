#include "ft_ping.h"

unsigned short checksum(unsigned short* ptr, int bytes)
{
    unsigned int sum = 0;

    for (sum = 0; bytes > 1; bytes -= 2)
        sum += *ptr++;
    if (bytes == 1)
        sum += *(unsigned char *)ptr;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (~sum);
}

void print_icmp_reply(char *reply, int bytes_received, struct sockaddr_in *reply_addr, struct timespec *send_time) {
    // Parse IP header
    struct iphdr *ip_hdr = (struct iphdr *)reply;
    int ip_header_len = ip_hdr->ihl * 4;

    // Parse ICMP header
    struct icmp *icmp_hdr = (struct icmp *)(reply + ip_header_len);

    if (icmp_hdr->icmp_type != ICMP_ECHOREPLY) {
        fprintf(stderr, "Non-echo reply received: type=%d code=%d\n", icmp_hdr->icmp_type, icmp_hdr->icmp_code);
        return;
    }

    // Calculate RTT
    struct timespec receive_time;
    clock_gettime(CLOCK_MONOTONIC, &receive_time);

    double rtt = (receive_time.tv_sec - send_time->tv_sec) * 1000.0; // seconds to ms
    rtt += (receive_time.tv_nsec - send_time->tv_nsec) / 1000000.0;  // nanoseconds to ms

    // Print output
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
           bytes_received - ip_header_len,
           inet_ntoa(reply_addr->sin_addr),
           icmp_hdr->icmp_seq,
           ip_hdr->ttl,
           rtt);
}

void    ft_ping(const char* target, int verbose)
{
    // check target
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, target, &addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid address: %s\n", target);
        exit(1);
    }

    // create socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // create packet
    char packet[64];
    struct icmp* icmp_packet = (struct icmp *)packet;
    icmp_packet->icmp_type = ICMP_ECHO;
    icmp_packet->icmp_code = 0;
    icmp_packet->icmp_cksum = 0;
    icmp_packet->icmp_id = getpid();
    icmp_packet->icmp_seq = 1;
    icmp_packet->icmp_cksum = checksum((unsigned short *)packet, sizeof(packet));

    // send packet
    struct timespec send_time;
    clock_gettime(CLOCK_MONOTONIC, &send_time);
    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) <= 0)
    {
        perror("sendto");
        close(sockfd);
        exit(1);
    }

    // receive reply
    char reply[1024];
    struct sockaddr_in reply_addr;
    socklen_t addr_len = sizeof(reply_addr);
    int bytes_received = recvfrom(sockfd, reply, sizeof(reply), 0, (struct sockaddr *) &reply_addr, &addr_len);
    if (bytes_received <= 0)
    {
        perror("recvfrom");
    }
    else
    {
        print_icmp_reply(reply, bytes_received, &reply_addr, &send_time);
    }

    close(sockfd);
}