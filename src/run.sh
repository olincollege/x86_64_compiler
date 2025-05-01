rm tokens && gcc *.c && ./a.out && as -o abcd.o chat.s && ld -o abcd abcd.o && ./abcd && echo $?
echo "Exit code: $?"
