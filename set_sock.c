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

void fill_stats(double diff_tv)
{
  //  g_env.op_ttl++;
    g_env.sum += diff_tv;
    if (diff_tv < g_env.min)
        g_env.min = diff_tv;
    if (diff_tv > g_env.max)
        g_env.max = diff_tv;
}

void sendpacket()
{
    printf("sendpacket\n");
    char *rawpacket;
    size_t rawpacket_size;
    struct sockaddr_in sock_in;

    memset(&sock_in, 0, sizeof(sock_in));
    sock_in.sin_family = AF_INET;
    sock_in.sin_addr.s_addr = g_env.res->sin_addr.s_addr;
    sock_in.sin_port = 0;
    memset(&(sock_in.sin_zero), 0, sizeof(sock_in.sin_zero));

    rawpacket = newpacket(&rawpacket_size);
    printf("goto sendto\n");
    if (sendto(g_env.sockfd, rawpacket, rawpacket_size, 0, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0)
    {
        perror("sendto error");
        printf("error sendto\n");
        //close(g_env.sockfd);
        //exit(EXIT_FAILURE);
    }
    printf("sendto\n");
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

/*void fill_rep_msg(struct s_reply *rep, int bytes_c, struct msghdr msg) {
    struct ip *ip  ;
    char *ptr;
    ssize_t ip_icmp_offset;

    if ((size_t)(bytes_c) < sizeof(struct ip)) {
        printf("error1\n");
        dprintf(2, "ft_ping: IP header from echo reply truncated\n");
        exit(2);
        // clean exit
    }
    rep->bytes_c = bytes_c;
    rep->msg_rep = msg;
    ptr = (char *)msg.msg_iov->iov_base;
    ip = (struct ip *)(char *)ptr;
    printf("test ip_p: %d\n", ip->ip_p);
    if (ip->ip_p != IPPROTO_ICMP){ //  || 
        printf("ip_p: %d\nIPPROT_ICMP: %d\n", ip->ip_p, IPPROTO_ICMP);
        dprintf(2, "ft_ping: IP header from echo reply truncated\n");
        printf("error2\n");
        exit(2);
        // clean exit
    }
    uint16_t ip_p_temp = ip->ip_p;
    uint16_t ip_hl_temp = ip->ip_hl;

    rep->icmp = (struct icmp *)(ptr + (ip_hl_temp << 2));
    printf("Icmp rep type: %d\n", rep->icmp->icmp_type);
    ip_icmp_offset = ip_hl_temp << 2;
   
    if ((size_t)(bytes_c) < ip_icmp_offset + sizeof(struct icmp)) {
        printf("ip_p: %d\nIPPROT_ICMP: %d\n", ip->ip_p, IPPROTO_ICMP);
        dprintf(2, "ft_ping: IP header from echo reply truncated\n");
        printf("error3\n");
        exit(2);
    }

    //rep->icmp = (struct icmp *)(ptr + ip_icmp_offset);
    printf("Icmp rep type: %d\n", rep->icmp->icmp_type);

}*/

void readpacket()
{
    printf("readpacket\n");

    char *ptr;
    struct ip *ip = NULL;
    struct icmp *icmp;
    struct msghdr msg;
    struct timeval tv_in;
    struct timeval *tv_out;
    double diff_tv;
    int bytes_c;

    struct s_reply rep;

    diff_tv = 0;
    printf("readpacket1\n");
    setbasemsghdr(&msg);
    printf("readpacket2\n");
    bytes_c = recvmsg(g_env.sockfd, &msg, 0);
    printf("bytes_c: %d\n", bytes_c);
    if (bytes_c > 0)
    {
        if (bytes_c < (int)sizeof(struct ip)) {
            printf("error1\n");
            dprintf(2, "ft_ping: IP header from echo reply truncated\n");
            exit(2);
            // clean exit
        }


        rep.bytes_c = bytes_c;
        printf("bytes_c: %d\n", bytes_c);
        ptr = (char *)msg.msg_iov->iov_base;
        ip = (struct ip *)ptr;
        if (ip->ip_p != IPPROTO_ICMP){ //  || 
            printf("ip_p: %d\nIPPROT_ICMP: %d\n", ip->ip_p, IPPROTO_ICMP);
            dprintf(2, "ft_ping: IP header from echo reply truncated\n");
            printf("error2\n");
            exit(2);
        // clean exit
        }
        if ((size_t)(bytes_c) < ip->ip_hl * 4 + sizeof(struct icmp)) {
           printf("ip_p: %d\nIPPROT_ICMP: %d\n", ip->ip_p, IPPROTO_ICMP);
         dprintf(2, "ft_ping: IP header from echo reply truncated\n");
            printf("error3\n");
            exit(2);
        }
        printf("Ip_p: %d\n", ip->ip_p);
        icmp = (struct icmp *)(ptr + (ip->ip_hl * 4));
        //fill_rep_msg(&rep, bytes_c, msg);
        //printf("icmp_type by rep: %d\n", rep.icmp->icmp_type);
        printf("icmp_type: %d\n reponse attendu : %d\n", icmp->icmp_type, ICMP_ECHOREPLY); 
        
        if (icmp->icmp_type != ICMP_ECHOREPLY) {
            printf("bad rep icmp type: %d\n", icmp->icmp_type);
            if (bytes_c >= 64) {
                icmp = (struct icmp *)(icmp->icmp_data) + sizeof(struct ip);
            }

            free(msg.msg_iov);
            return ;
        }
        gettimeofday(&tv_in, NULL);
            tv_out = (struct timeval *)(icmp + 1);
            if (tv_out && tv_out->tv_sec && tv_out->tv_usec)
            {
                //printf("tv_in.tv_sec: %ld, tv_in.tv_usec: %ld\n", tv_in.tv_sec, tv_in.tv_usec);
               // printf("tv_out->tv_sec: %ld, tv_out->tv_usec: %ld\n", tv_out->tv_sec, tv_out->tv_usec);

                diff_tv = (tv_in.tv_sec - tv_out->tv_sec) * 1000.0 + (tv_in.tv_usec - tv_out->tv_usec) / 1000.0;
                if (tv_out != NULL && tv_out->tv_sec != 0 && tv_out->tv_usec != 0)
                {
                    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", bytes_c, inet_ntoa(ip->ip_src), icmp->icmp_seq, ip->ip_ttl, diff_tv);
                    fill_stats(diff_tv);
                   // printf("%d bytes from %s: icmp_seq=%d ttl=%d time= ms\n", bytes_c, inet_ntoa(ip->ip_src), icmp->icmp_seq, ip->ip_ttl);
                }
            }
        //  printf("%d bytes from : time=%.3f ms\n", bytes_c, diff_tv);
        //*/
        
    }
    else if (bytes_c < 0)
    {
        printf("test1\n");
        perror("recvmsg");
    }
    else if (bytes_c == 0)
    {
        dprintf(2, "ft_ping: socket closed\n");
        //exit clean;
        printf("test2\n");
    }
    else if (bytes_c >= (int)(sizeof(struct icmp *)) + (sizeof(struct ip)))
    {

        printf(" test 2 bytes_c: %d\n", bytes_c);
        struct icmp *err_icmp;
        //err_icmp = 
    }
    else
    {
        printf(" test 3 bytes_c: %d\n", bytes_c);
    }
    free(msg.msg_iov);
}

int set_sock()
{
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    g_env.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (g_env.sockfd < 0)
    {
        perror("socket");
        return (EXIT_FAILURE);
    }
    setsockopt(g_env.sockfd, SOL_SOCKET, SO_REUSEADDR, &(g_env.op_reuseaddr), sizeof(g_env.op_reuseaddr));
    setsockopt(g_env.sockfd, IPPROTO_IP, IP_TTL, &(g_env.op_ttl), sizeof(g_env.op_ttl));
    setsockopt(g_env.sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    gettimeofday(&(g_env.start_time), NULL);
    g_env.seq_num = 0;
    signal(SIGINT, sigint_handler);
    while (1)
    {
        sendpacket();
        readpacket();
        usleep(1000000);
    }
    return (1);
}