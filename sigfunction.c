#include "ft_ping.h"

extern t_env g_env;

void sigint_handler(int sig)
{

    (void)sig;

    gettimeofday(&g_env.end_time, NULL);
    printf("\n--- %s ping statistics ---\n", g_env.ipstr);
    printf("%d packets transmitted, %d packets received, %.1f%% packet loss, time %.0fms\n", g_env.seq_num, g_env.valide_ping, (double)((g_env.seq_num - g_env.valide_ping) * 100) / (double)g_env.seq_num, (double)(g_env.end_time.tv_sec - g_env.start_time.tv_sec) * 1000 + (double)(g_env.end_time.tv_usec - g_env.start_time.tv_usec) / 1000);
    printf("rtt min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", g_env.min, g_env.sum / (double)g_env.seq_num, g_env.max, get_stddev());
    
    // need last stat here 
    exit(0);
    // exit properly
}