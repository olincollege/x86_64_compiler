rm tokens && gcc *.c && ./a.out && as -o test.o chat.s && ld -o test test.o && clear && ./test && echo $?
echo "Return Value: $?"
