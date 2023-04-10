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

# define ICMP_DATA_SIZE 56

typedef struct s_icmp_packet {
    struct icmp icmp;
    char data[56];
}       icmp_packet;

typedef struct s_env {

    int sockfd;
    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in *res;
    int timeout;

    int op_verbose;

    double min;
    double max;
    double sum;
}               t_env;

int pars_arg(int ac, char **av);
int set_sock(void);

#endif

