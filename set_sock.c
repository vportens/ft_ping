#include "ft_ping.h"

extern t_env g_env;

unsigned short checksum(unsigned short *buf, int bufsz)
{
    unsigned long sum = 0;
    unsigned short result;

    while (bufsz > 1)
    {
        sum += *buf++;
        bufsz -= 2;
    }

    if (bufsz == 1)
    {
        sum += *(unsigned char *)buf;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = ~sum;
    return (result);
}

void set_icmp(char *rawpacket, size_t payload_s)
{
    struct icmp *icmp;

    icmp = (struct icmp *)rawpacket;
    icmp->icmp_type = 8;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_id = getpid();
    icmp->icmp_seq = (g_env.seq_num++);
    icmp->icmp_cksum = checksum((unsigned short *)icmp, sizeof(struct icmp) + payload_s);
}

char *newpacket(size_t *size)
{
    char *rawpacket;
    size_t payload_s;
    struct timeval tv;

    payload_s = sizeof(struct timeval);
    *size = sizeof(struct icmp) + payload_s;
    rawpacket = malloc(sizeof(char) * (*size));
    memset(rawpacket, 0, *size);
    gettimeofday(&tv, NULL);
    memcpy(rawpacket + sizeof(struct icmp), &tv, sizeof(struct timeval)); //
    set_icmp(rawpacket, payload_s);
    return (rawpacket);
}

void setIcmpVoid(struct icmp *icmp)
{

    icmp->icmp_type = 0;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_id = 0;
    icmp->icmp_seq = 0;
    icmp->icmp_cksum = 0;
}

void sendpacket()
{
    char *rawpacket;
    size_t rawpacket_size;
    struct sockaddr_in sock_in;

    memset(&sock_in, 0, sizeof(sock_in));
    sock_in.sin_family = AF_INET;
    sock_in.sin_addr.s_addr = g_env.res->sin_addr.s_addr;
    sock_in.sin_port = 0;
    memset(&(sock_in.sin_zero), 0, sizeof(sock_in.sin_zero));

    rawpacket = newpacket(&rawpacket_size);
    if (sendto(g_env.sockfd, rawpacket, rawpacket_size, 0, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0)
    {
        perror("sendto");
        close(g_env.sockfd);
        exit(EXIT_FAILURE);
    }

    free(rawpacket);
}

void *ft_memset(void *b, int c, size_t len)
{
    unsigned char *ch_b;

    ch_b = b;
    while (len--)
        *ch_b++ = (unsigned char)c;
    return (b);
}

void setbasemsghdr(struct msghdr *msg)
{
    struct iovec *iov;
    char recvbuf[100];

    iov = malloc(sizeof(struct iovec));
    ft_memset(msg, 0, sizeof(msg));
    ft_memset(iov, 0, sizeof(*iov));

    iov->iov_base = recvbuf;
    iov->iov_len = sizeof(recvbuf);
    msg->msg_name = NULL;
    msg->msg_namelen = 0;
    msg->msg_iov = iov;
    msg->msg_iovlen = 1;
    msg->msg_control = NULL;
    msg->msg_controllen = 0;
    msg->msg_flags = 0;
}

void readpacket()
{

    char *ptr;
    struct ip *ip = NULL;
    struct icmp *icmp;
    struct msghdr msg;
    struct timeval tv_in;
    struct timeval *tv_out;
    double diff_tv;
    long unsigned int bytes_c;

    diff_tv = 0;
    setbasemsghdr(&msg);
    if ((bytes_c = recvmsg(g_env.sockfd, &msg, 0)) > 0)
    {
        ptr = (char *)msg.msg_iov->iov_base;
        ip = (struct ip *)ptr;
        icmp = (struct icmp *)(ptr + (ip->ip_hl * 4));
        gettimeofday(&tv_in, NULL);
            tv_out = (struct timeval *)(icmp + 1);
            if (tv_out && tv_out->tv_sec && tv_out->tv_usec)
            {
                //printf("tv_in.tv_sec: %ld, tv_in.tv_usec: %ld\n", tv_in.tv_sec, tv_in.tv_usec);
               // printf("tv_out->tv_sec: %ld, tv_out->tv_usec: %ld\n", tv_out->tv_sec, tv_out->tv_usec);

                diff_tv = (tv_in.tv_sec - tv_out->tv_sec) * 1000.0 + (tv_in.tv_usec - tv_out->tv_usec) / 1000.0;
                if (tv_out != NULL && tv_out->tv_sec != 0 && tv_out->tv_usec != 0)
                {
                    printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", bytes_c, inet_ntoa(ip->ip_src), icmp->icmp_seq, ip->ip_ttl, diff_tv);
                   // printf("%d bytes from %s: icmp_seq=%d ttl=%d time= ms\n", bytes_c, inet_ntoa(ip->ip_src), icmp->icmp_seq, ip->ip_ttl);
                }
            }
        //  printf("%d bytes from : time=%.3f ms\n", bytes_c, diff_tv);
    }
    else
    {
        perror("recvmsg");
    }
    free(msg.msg_iov);
}

int set_sock()
{
    g_env.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (g_env.sockfd < 0)
    {
        perror("socket");
        return (EXIT_FAILURE);
    }
    setsockopt(g_env.sockfd, SOL_SOCKET, SO_REUSEADDR, &(g_env.op_reuseaddr), sizeof(g_env.op_reuseaddr));
    setsockopt(g_env.sockfd, IPPROTO_IP, IP_TTL, &(g_env.op_ttl), sizeof(g_env.op_ttl));
    gettimeofday(&(g_env.start_time), NULL);
    g_env.seq_num = 0;
    int i = 0;
    signal(SIGINT, sigint_handler);
    while (1)
    {
        sendpacket();
        readpacket();
        usleep(1000000);
        i++;
        if (i == 5)
            break;
    }
    return (1);
}