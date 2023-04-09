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
 
    int seq_sum = 1;

    while (1) {
       

      

       memset(&iov, 0, sizeof(iov));
       memset(&msg, 0, sizeof(msg));
       memset(&icmp, 0, sizeof(icmp));
      // memset(icmp_data, 0, ICMP_DATA_SIZE);

        icmp.icmp_type = ICMP_ECHO;
        icmp.icmp_code = 0;
        icmp.icmp_id = getpid();
        icmp.icmp_seq = seq_sum++;
        gettimeofday((struct timeval *)icmp.icmp_data, NULL);
        icmp.icmp_cksum = 0;
        icmp.icmp_cksum = checksum((unsigned short *)&icmp, sizeof(icmp));


        iov.iov_base = buf;
        iov.iov_len = sizeof(buf);

        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        msg.msg_flags = 0;

        if (sendto(g_env.sockfd, &icmp, sizeof(icmp), 0, (struct sockaddr *)g_env.res, sizeof(struct sockaddr)) < 0) {
            perror("sendto");
            close(g_env.sockfd);
            exit(EXIT_FAILURE);
        }
 


        int nbytes = recvmsg(g_env.sockfd, &msg, 0);

        if (nbytes < 0) {
            if (errno == EAGAIN) {
                printf("Request timeout for icmp_seq \n");
            } else {
                perror("recvfrom");
            }
            continue;
        }

        ip = (struct ip *)buf;
        icmp_rec = (struct icmp *)(buf + (ip->ip_hl << 2));
        gettimeofday(&in, NULL);
        out = (struct timeval *)(icmp_rec->icmp_data);

        double latency = (in.tv_sec - out->tv_sec) * 1000 + (in.tv_usec - out->tv_usec) / 1000;
        //printf("Received ICMP packet from %s: icmp_seq=%d  time=%f ms\n", g_env.ipstr, icmp.icmp_seq,  latency);
        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%f ms\n", nbytes  , g_env.ipstr, icmp.icmp_seq, ip->ip_ttl, latency);
       usleep(1000000);
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