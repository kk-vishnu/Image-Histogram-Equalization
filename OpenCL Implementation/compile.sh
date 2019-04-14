gcc readimg.c -o readimg -lm
gcc main_code.c -lOpenCL -L$AMDAPPSDKROOT/lib/x86_64 -o op
gcc writeimg.c -o writeimg -lm
