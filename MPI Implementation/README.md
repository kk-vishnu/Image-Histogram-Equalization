To compile:
./compile.sh


To run:
./run.sh 16 
NOTE: Choose a number that divides the number of pixels




Stepwise execution:

./readimg
mpiexec -n $1 ./freq 
NOTE: Choose a number that divides the number of pixels

mpiexec -n 4 ./prob 
NOTE: Number of processes should evenly divide 256. It will wait for input,enter maximum intensity needed

mpiexec -n $1 ./contrast 
Choose a number that divides the number of pixels

./writeimg
