#include "ft_ping.h"

extern t_env g_env;

void sigint_handler(int sig)
{

    (void)sig;

    gettimeofday(&g_env.end_time, NULL);
    printf("\n--- %s ping statistics ---\n", g_env.ipstr);
    printf("%d packets transmitted, %d received, %.1f%% packet loss, time %.0fms\n", g_env.seq_num, g_env.seq_num - g_env.op_ttl, (double)(g_env.op_ttl * 100) / (double)g_env.seq_num, (double)(g_env.end_time.tv_sec - g_env.start_time.tv_sec) * 1000 + (double)(g_env.end_time.tv_usec - g_env.start_time.tv_usec) / 1000);
    
    // need last stat here 
    // exit properly
}