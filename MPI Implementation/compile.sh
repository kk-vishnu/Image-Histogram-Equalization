gcc readimg.c -o readimg -lm
mpicc freq.c -o freq
mpicc prob.c -o prob
mpicc contrast.c -o contrast
gcc writeimg.c -o writeimg -lm
