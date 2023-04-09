#include "ft_ping.h"

extern t_env g_env;

int pars_hostname(char *target) {
    struct addrinfo hints;
    struct addrinfo *res;
    int             status;


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(target, NULL, &hints, &res)) != 0) {
        fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(status));
        return (EXIT_FAILURE);
    }

    printf("address ip : %s\n", target);

    void *addr;
    char *ipver;

    if (res->ai_family == AF_INET) { // IPv4
        g_env.res = (struct sockaddr_in *)res->ai_addr;
        addr = &((g_env.res)->sin_addr);
        ipver = "IPv4";
    } else { // IPv6
        fprintf(stderr, "Error IPv6 not supported\n");
        freeaddrinfo(res);
        return (EXIT_FAILURE);
   }

    if (inet_ntop(res->ai_family, addr, g_env.ipstr, sizeof g_env.ipstr) == NULL) {
        perror("inet_ntop");
        freeaddrinfo(res);
        return (EXIT_FAILURE);
    }
    printf("  %s: %s\n", ipver, g_env.ipstr);
    freeaddrinfo(res);
    return (0);
}

int pars_arg(int ac, char **av) {
    char *arg;
    int i = 1;
    int j = 1;

    if (ac < 2) {
       return (printf("Error nbr arg\nUsage: ./ft_ping [-v] [-h] target\n" ));
    }
    if (strcmp(av[0], "./ft_ping") != 0) {
       return (printf("Error program name\nUsage: ./ft_ping [-v] [-h] target\n" ));
    }
    
    while (i < ac) {
        arg = av[i];
        if (arg[0] == '-') {
            if (strlen(arg) < 2) {
                printf("Error option not found\n./ft_ping [-v] [-h] target\n");
                exit(1);
            }
            while (arg[j] != '\0') {
                if (arg[j] == 'h') {
                    printf("\nUsage\n\tping [option] <destination>\n\nOptions:\n\t<destination>\t\tdns name or ip address\n\t-h\t\tprint help and exit\n\t-v\t\tverbose output\n");
                    exit(0);
                }
                else if (arg[j] == 'v') {
                    g_env.op_verbose = 1;
                    // active verbose mode
                }
                else {
                    printf("Error option not found\n./ft_ping [-v] [-h] target\n");
                    exit(EXIT_FAILURE);
                }
                j++;
            }
        }
        else {
            // catch ip/hostname
            pars_hostname(arg);
        }
        i++;
    }
    return (printf("end pars_arg\n"));
}