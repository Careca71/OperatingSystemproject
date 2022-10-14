
BININC	= ./include 
BININ	= ./bin
BINSRC	= ./src
BINLIB  = ./lib
BINTEST = ./testfile
CC		=  gcc
AR              =  ar
CFLAGS	        += -std=c99 -Wall -g
ARFLAGS         =  rvs
INCLUDES	= -I./include
LIBFLAGS= -L./lib
LIBS= -lSqueue
THR= -lpthread
OPTFLAGS	= -O3 -DNDEBUG 


.PHONY: all clean cleanall test
.SUFFIXES: .c .h

$(BININ)/%.o:$(BINSRC)/%.c 
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

$(BININ)/supermercato:$(BININ)/supermercato.o $(BINLIB)/libSqueue.a
	$(CC) $(CCFLAGS) $(INCLUDES) $(OPTFLAGS)  -o $@ $^ $(LIBFLAGS) $(THR) $(LIBS)

$(BINLIB)/libSqueue.a:  $(BININ)/cashier.o $(BININ)/director.o $(BININ)/sharedlib.o $(BININ)/client.o 
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -f super.txt
	rm -f $(BININ)/*.o
	rm -f $(BINLIB)/*.a
	rm -f $(BININ)/supermercato
	rm -f $(BINTEST)/client.txt
	rm -f $(BINTEST)/cashier.txt
	rm -f $(BINTEST)/direttore.txt

test:
	./$(BININ)/supermercato $(BINTEST)/config.txt > $ super.txt &
	sleep 25
	killall -1 supermercato
	bash testfile/analisi.sh super.txt
