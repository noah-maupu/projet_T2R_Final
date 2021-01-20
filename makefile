PROGRAMS = clientAPI TicketToRideAPI main

.PHONY: all clean install

all: $(PROGRAMS)

clean: 
	rm -f $(PROGRAMS)



clientAPI: clientAPI.c
	gcc -c clientAPI.c

TicketToRideAPI: TicketToRideAPI.c
	gcc -c TicketToRideAPI.c

main: main.c clientAPI.c TicketToRideAPI.c
	gcc -Wall -o main main.c clientAPI.o TicketToRideAPI.o
