tcp: tcp_receive tcp_send

tcp_receive: tcp_receive.o
	gcc -w ./tcp_receive.c -o tcp_receive

tcp_send: tcp_send.o
	gcc -w ./tcp_send.c -o tcp_send

clean:
	-rm *.o
	-rm tcp_send
	-rm tcp_receive