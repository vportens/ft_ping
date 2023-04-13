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

    


/*
void inf_loop() {

    struct icmp isend;
    unsigned char recvbuf[84];

 
    
  int seq_num = 1;
while (1) {
    // Étape 1 : Créez et envoyez un paquet ICMP Echo Request
    memset(&isend, 0, sizeof(isend) - sizeof(isend.icmp_data) + ICMP_DATA_SIZE);
    isend.icmp_type = ICMP_ECHO;
    isend.icmp_code = 0;
    isend.icmp_id = htons(getpid());
    isend.icmp_seq = htons(seq_num++);

    // Remplissez les données ICMP
    gettimeofday((struct timeval *)isend.icmp_data , NULL);
    isend.icmp_cksum = 0;
    isend.icmp_cksum = checksum((unsigned short *)&isend, sizeof(isend) - sizeof(isend.icmp_data) + ICMP_DATA_SIZE);

    // Envoyez le paquet ICMP Echo Request
    size_t isend_size = sizeof(isend);

    if (sendto(g_env.sockfd, &isend, isend_size, 0, (struct sockaddr *)g_env.res, sizeof(struct sockaddr)) < 0) {
        perror("sendto");
        close(g_env.sockfd);
        exit(EXIT_FAILURE);
    }

    // Étape 2 : Réception du paquet ICMP Echo Reply
    struct msghdr msg;
    struct iovec iov;
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
    msg.msg_control = &g_env.ctrl;
    msg.msg_controllen = sizeof(g_env.ctrl);

    // Étape 3 : Attendre et vérifier le paquet ICMP Echo Reply
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(g_env.sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    ssize_t recvmsg_size = recvmsg(g_env.sockfd, &msg, 0);
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
    int ip_header_len = ip_header->ip_hl * 4;
    printf("ip_header_len: %d\n", ip_header_len);
    printf("ip header: %d\n", ntohs(ip_header->ip_len));
    struct icmp *icmp_recv = (struct icmp *)(recvbuf + ip_header_len);

    if (icmp_recv->icmp_type == ICMP_ECHOREPLY) {
        struct timeval cur_time;
        gettimeofday(&cur_time, NULL);
        struct timeval *recv_time = (struct timeval *)icmp_recv->icmp_data;
        double rtt = (cur_time.tv_sec - recv_time->tv_sec) * 1000.0 + (cur_time.tv_usec - recv_time->tv_usec) / 1000.0;
        printf("%zd bytes from rtt: %f \n", recvmsg_size, rtt);
    }
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


*/

void set_icmp(char *rawpacket, size_t payload_s) {
    struct icmp *icmp;

    icmp = (struct icmp *)rawpacket;
    icmp->icmp_type = 8;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_id = getpid();
    icmp->icmp_seq = (g_env.seq_num++);
    icmp->icmp_cksum = checksum((unsigned short *)icmp, sizeof(struct icmp) + payload_s);
}

char *newpacket(size_t *size) {
    char *rawpacket;
    size_t payload_s;
    struct timeval tv;

    payload_s = sizeof(struct timeval);
    *size = sizeof(struct icmp) + payload_s;
    rawpacket = (char *)malloc((*size));
    memset(rawpacket, 0, *size);
    gettimeofday(&tv, NULL);
    set_icmp(rawpacket, payload_s);
    return (rawpacket);
}

void sendpacket() {
    char *rawpacket;
    size_t rawpacket_size;
    struct sockaddr_in sock_in;

    sock_in.sin_family = AF_INET;
    sock_in.sin_addr.s_addr = g_env.res->sin_addr.s_addr;
    memset(&(sock_in.sin_zero), 0, sizeof(sock_in.sin_zero));

    rawpacket = newpacket(&rawpacket_size);
    if (sendto(g_env.sockfd, rawpacket, rawpacket_size, 0, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0) {
        perror("sendto");
        close(g_env.sockfd);
        exit(EXIT_FAILURE);
    }

    free(rawpacket);
}

void	*ft_memset(void *b, int c, size_t len)
{
	unsigned char *ch_b;

	ch_b = b;
	while (len--)
		*ch_b++ = (unsigned char)c;
	return (b);
}


void setbasemsghdr(struct msghdr *msg) {
    struct iovec iov;
    char recvbuf[100];

    ft_memset(msg, 0, sizeof(msg));
    ft_memset(&iov, 0, sizeof(iov));

    iov.iov_base = recvbuf;
    iov.iov_len = sizeof(recvbuf);
    msg->msg_name = NULL;
    msg->msg_namelen = 0;
    msg->msg_iov = &iov;
    msg->msg_iovlen = 1;
    msg->msg_control = NULL;
    msg->msg_controllen = 0;
    msg->msg_flags = 0;
}

void readpacket() {

    char *ptr;
    struct ip *ip;
    struct icmp *icmp;
    struct msghdr msg;
    struct timeval tv_in;
    struct timeval *tv_out;
    double diff_tv;
    int bytes_c;

    setbasemsghdr(&msg);
    if ((bytes_c = recvmsg(g_env.sockfd, &msg, 0)) > 0) {
        printf("test4, bytesc %d\n", bytes_c);
        ptr = (char *)msg.msg_iov->iov_base;
        ip = (struct ip *)ptr;
        icmp = (struct icmp *)(ptr + (ip->ip_hl * 4));
        gettimeofday(&tv_in, NULL);
        tv_out = (struct timeval *)(icmp + 1);
        diff_tv = (tv_in.tv_sec - tv_out->tv_sec) * 1000.0 + (tv_in.tv_usec - tv_out->tv_usec) / 1000.0;
        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms", bytes_c, inet_ntoa(ip->ip_src), icmp->icmp_seq, ip->ip_ttl, diff_tv);
    }
    else {
        perror("recvmsg");
    }

}

int set_sock() {
    g_env.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (g_env.sockfd < 0) {
        perror("socket");
        return (EXIT_FAILURE);
    }
    setsockopt(g_env.sockfd, SOL_SOCKET, SO_REUSEADDR, &(g_env.op_reuseaddr), sizeof(g_env.op_reuseaddr));
    setsockopt(g_env.sockfd, IPPROTO_IP, IP_TTL, &(g_env.op_ttl), sizeof(g_env.op_ttl));
    gettimeofday(&(g_env.start_time), NULL);
    g_env.seq_num = 1;
    while (1) {
        sendpacket();
        readpacket();
    }
}