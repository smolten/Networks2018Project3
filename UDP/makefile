udp: udp_receive udp_send

udp_receive: udp_receive.o
	gcc ./udp_receive.c -o udp_receive

udp_send: udp_send.o
	gcc ./udp_send.c -o udp_send

clean:
	-rm *.o
	-rm udp_send
	-rm udp_receive