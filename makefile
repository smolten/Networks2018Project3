udp: udp_rcv udp_snd

udp_rcv: udp_receive.o
	gcc -o udp_rcv udp_receive.o

udp_receive.o: udp_receive.c
	gcc -c udp_receive.c

udp_snd: udp_send.o
	gcc -o udp_snd udp_send.o

udp_send.o: udp_send.c
	gcc -c udp_send.c

clean:
	rm *.o
	rm a.out
	rm udp_snd
	rm udp_rcv