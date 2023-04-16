#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <netdb.h>
# include <errno.h>
# include <signal.h>

# define ICMP_DATA_SIZE 56


typedef struct s_env {

    int sockfd;
    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in *res;
    int timeout;

    int seq_num;

    int op_ttl;
    int op_reuseaddr;
    int op_verbose;

    double min;
    double max;
    double sum;
    double *lst_stat;

    struct timeval start_time;
    struct timeval end_time;

}               t_env;

typedef struct s_reply {
    struct ip ip_rep;
    struct icmp *icmp_rep;
    struct msghdr msg_rep;
    int bytes_c;

}       t_reply;

int pars_arg(int ac, char **av);
int set_sock(void);
void sigint_handler(int sig);

#endif

