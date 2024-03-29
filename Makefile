CC=g++
LIBSOCKET=-lnsl
CCFLAGS=-Wall -g
SRV=server
SEL_SRV=server
CLT=client

all: $(SEL_SRV) $(CLT)

$(SEL_SRV):$(SEL_SRV).cpp
	$(CC) -fpermissive -o $(SEL_SRV) $(LIBSOCKET) $(SEL_SRV).cpp

$(CLT):	$(CLT).cpp
	$(CC) -o $(CLT) $(LIBSOCKET) $(CLT).cpp

clean:
	rm -f *.o *~
	rm -f $(SEL_SRV) $(CLT)
