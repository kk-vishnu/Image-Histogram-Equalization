To compile:
./compile.sh

To run:
./solution.sh


Stepwise execution:

gcc readimg.c -o readimg -lm

./readimg

gcc main_code.c -lOpenCL -L$AMDAPPSDKROOT/lib/x86_64 -o op

./op

gcc writeimg.c -o writeimg -lm

./writeimg


Final image is stored as finalimage.png
