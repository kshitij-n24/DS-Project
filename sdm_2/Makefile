CC = g++
CFLAGS = -Wall
DEPS = headers.h  
OBJ = classes/Logger.o classes/ServerSocket.o classes/ClientSocket.o classes/Utils.o classes/SDMService.o sdm.o

%.o: %.cpp $(DEPS)
    $(CC) $(CFLAGS) -c -o $@ $<

sdm: $(OBJ)
    $(CC) $(CFLAGS) -pthread -o $@ $^

clean:
    rm -rf *.o sdm ./classes/*.o