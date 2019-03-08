PORT=57645
CFLAGS= -DPORT=\$(PORT) -g -Wall -std=gnu99

hcq_server: hcq_server.o hcq.o
	gcc $(CFLAGS) -o hcq_server hcq_server.o hcq.o

hcq_server.o: hcq_server.c hcq.h
	gcc $(CFLAGS) -c hcq_server.c

helpcentre: helpcentre.o hcq.o
	gcc $(CFLAGS) -o helpcentre helpcentre.o hcq.o

helpcentre.o: helpcentre.c hcq.h
	gcc $(CFLAGS) -c helpcentre.c

hcq.o: hcq.c hcq.h
	gcc $(CFLAGS) -c hcq.c

clean:
	rm -f *.o hcq_server helpcentre
