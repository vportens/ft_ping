#include "ft_ping.h"

extern t_env g_env;

unsigned short checksum(unsigned short *buf, int bufsz) {
    unsigned long sum = 0;
    unsigned short result;

    while (bufsz > 1) {
        sum += *buf++;
        bufsz -= 2;
    }

    if (bufsz == 1) {
        sum += *(unsigned char *)buf;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = ~sum;
    return (result);
}

    



void inf_loop() {
    struct msghdr msg;
    struct iovec iov;
    struct icmp icmp;
    struct icmp *icmp_rec;
    char buf[1500];
    struct timeval in;
    struct timeval *out;
    struct ip *ip;
 
  int seq_num = 1;
while (1) {
    // Étape 1 : Créez et envoyez un paquet ICMP Echo Request
    struct icmp_packet icmp_send;
    memset(&icmp_send, 0, sizeof(icmp_send));
    icmp_send.header.icmp_type = ICMP_ECHO;
    icmp_send.header.icmp_code = 0;
    icmp_send.header.icmp_id = htons(getpid());
    icmp_send.header.icmp_seq = htons(seq_num++);

    // Remplissez les données ICMP
    gettimeofday((struct timeval *)icmp_send.data, NULL);
    icmp_send.header.icmp_cksum = 0;
    icmp_send.header.icmp_cksum = in_cksum((unsigned short *)&icmp_send, sizeof(icmp_send));

    // Envoyez le paquet ICMP Echo Request
    size_t icmp_send_size = sizeof(icmp_send);
    if (sendto(sockfd, &icmp_send, icmp_send_size, 0, res->ai_addr, res->ai_addrlen) < 0) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Étape 2 : Réception du paquet ICMP Echo Reply
    struct msghdr msg;
    struct iovec iov;
    unsigned char recvbuf[84];
    struct sockaddr_in recv_addr;

    memset(&msg, 0, sizeof(msg));
    memset(&recv_addr, 0, sizeof(recv_addr));
    memset(&iov, 0, sizeof(iov));
    memset(recvbuf, 0, sizeof(recvbuf));

    iov.iov_base = recvbuf;
    iov.iov_len = sizeof(recvbuf);
    msg.msg_name = &recv_addr;
    msg.msg_namelen = sizeof(recv_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // Étape 3 : Attendre et vérifier le paquet ICMP Echo Reply
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    ssize_t recvmsg_size = recvmsg(sockfd, &msg, 0);
    if (recvmsg_size < 0) {
        if (errno == EAGAIN) {
            printf("Request timeout for icmp_seq %d\n", seq_num - 1);
        } else {
            perror("recvmsg");
        }
        continue;
    }

    // Traitez le paquet ICMP Echo Reply
    struct ip *ip_header = (struct ip *)recvbuf;
    int ip_header_len = ip_header->ip_hl << 2;
    struct icmp *icmp_recv = (struct icmp *)(recvbuf + ip_header_len);

    if (icmp_recv->icmp_type == ICMP_ECHOREPLY) {
        struct timeval cur_time;
        gettimeofday(&cur_time, NULL);
        struct timeval *recv_time = (struct timeval *)icmp_recv->icmp_data;
        double rtt = (cur_time.tv_sec - recv_time->tv_sec) * 1000.0 + (cur_time.tv_usec - recv_time->tv_usec) / 1000.0;
        printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n", recvmsg_size, ipstr
    }
    return ;
}

int set_sock() {
    g_env.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (g_env.sockfd < 0) {
        perror("socket");
        return (EXIT_FAILURE);
    }
    g_env.timeout = 1;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(g_env.sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)) < 0) {
        perror("setsockopt");
        return (EXIT_FAILURE);
    }

    inf_loop();
    return (0);
}