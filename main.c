#include "ft_ping.h"

t_env g_env;

int main(int ac, char **av) {
    // init g_env //
   pars_arg(ac, av); 
   set_sock();
   free(g_env.res);
}