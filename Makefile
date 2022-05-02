CFLAGS = -Wall -g

POMOCNI_C = pomocni.c
POMOCNI_O =${POMOCNI_C:.c=.o}

PRVI_C = CandC.c
PRVI_O = ${PRVI_C:.c=.o}

DRUGI_C = server.c
DRUGI_O = ${DRUGI_C:.c=.o}

TRECI_C = bot.c
TRECI_O = ${TRECI_C:.c=.o}

all: CandC bot server

CandC: $(PRVI_O) $(POMOCNI_O)
	$(CC) $(PRVI_O) $(POMOCNI_O) -o $@
	
server: $(DRUGI_O) $(POMOCNI_O)
	$(CC) $(DRUGI_O) $(POMOCNI_O) -o $@
	
bot: $(TRECI_O) $(POMOCNI_O)
	$(CC) $(TRECI_O) $(POMOCNI_O) -o $@

clean:
	-rm -f CandC bot server *.o *.core
