udp: udp_rcv udp_snd

udp_rcv: udp_receive.o
	g++ -o udp_rcv udp_receive.o

udp_receive.o: udp_receive.c
	g++ -c udp_receive.c

udp_snd: udp_send.o
	g++ -o udp_snd udp_send.o

udp_send.o: udp_send.c
	g++ -c udp_send.c

clean:
	rm *.o
	rm a.out