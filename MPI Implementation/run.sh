./readimg
mpiexec -n $1 ./freq
mpiexec -n 4 ./prob
mpiexec -n $1 ./contrast
./writeimg
rm config
rm file1
rm file2
rm finalpixels
rm op1
rm time_calc
