OBJ = main.o \
	Configure.o \
	DaqControl.o \
	TofPetAsic.o \
	Lin_TcpSocket.o

DEPS = Configure.h \
	DaqControl.h \
	TofPetAsic.h \
	Lin_TcpSocket.h

EXE=Daq
CC=g++ 
#CFLAGS=-c -Wall -DLINUX -DTCP -g
CFLAGS=-c -Wall -DLINUX -g
LDFLAGS=-L. -lconfig++

$(EXE): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

.cpp.o: $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -f $(EXE) $(OBJ) *~ *.log

