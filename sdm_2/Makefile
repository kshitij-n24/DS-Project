CC = g++
CFLAGS = -Wall -Werror -pedantic
DEPS = headers.h  
OBJ = classes/Logger.o classes/Groups.o classes/ServerSocket.o classes/Utils.o classes/SDM.o SDM.o

%.o: %.cpp $(DEPS)
    $(CC) $(CFLAGS) -c -o $@ $<

sdm: $(OBJ)
    $(CC) $(CFLAGS) -pthread -o $@ $^

clean:
    rm -rf *.o sdm ./classes/*.o