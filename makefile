CFLAGS = -Wall -c
LFLAGS = -Wall
CC = gcc
LOC = /usr/lib 
OBJS = main.o myqueue.o tcpsocket.o datamgr.o storagemgr.o gateway.o 
EXE = sensor_gateway
CPPCHECK = cppcheck
LIB = list


all: sensor_gateway sensor_node

########################  sensor gateway, also as a FIFO writer ########################

run_sensor_gateway: $(EXE)
	./$(EXE)

sensor_gateway: $(OBJS)
	$(CC) $(LFLAGS) -pg $(OBJS) -L. -Wl,-rpath=. -l$(LIB) -pthread -o $(EXE) 

main.o: main.c common.h
	$(CC) $(CFLAGS) -pg main.c  

datamgr.o: datamgr.c datamgr.h common.h
	$(CC) $(CFLAGS) -pg datamgr.c -DSET_MAX_TEMP=21 -DSET_MIN_TEMP=18

storagemgr.o: storagemgr.c storagemgr.h common.h
	$(CC) $(CFLAGS) -pg storagemgr.c 
	
gateway.o: gateway.c gateway.h common.h
	$(CC) $(CFLAGS) -pg gateway.c -DTIMEOUT=5

myqueue.o:myqueue.c myqueue.h
	$(CC) $(CFLAGS) -pg myqueue.c

# not supported
# sensor_db.o:sensor_db.c sensor_db.h
	# $(CC) $(CFLAGS) -pg sensor_db.c `mysql_config --cflags --libs`


tcpsocket.o:tcpsocket.c tcpsocket.h
	$(CC) $(CFLAGS) -pg tcpsocket.c



###########################    sensor node   ##############################
sensor_node:sensor_node.o tcpsocket.o
	$(CC) $(LFLAGS) -pg sensor_node.o tcpsocket.o -o sensor_node 

# -DLOOPS=10, without defining LOOPS, it is infinite loop
sensor_node.o:sensor_node.c 
	$(CC) $(CFLAGS) -pg sensor_node.c -DLOOPS=5

sensor_node1: sensor_node
	./sensor_node 1 1 127.0.0.1 1234 &

sensor_node2: sensor_node
	./sensor_node 2 1 127.0.0.1 1234 &

sensor_node3: sensor_node
	./sensor_node 3 1 127.0.0.1 1234 &

sensor_node4: sensor_node
	./sensor_node 4 1 127.0.0.1 1234 &

sensor_node5: sensor_node
	./sensor_node 5 1 127.0.0.1 1234 &

sensor_node6: sensor_node
	./sensor_node 6 1 127.0.0.1 1234 &

sensor_node7: sensor_node
	./sensor_node 7 1 127.0.0.1 1234 &

sensor_node8: sensor_node
	./sensor_node 8 1 127.0.0.1 1234 &



###########################   FIFO reader  ####################################

fifo_reader.o: fifo_reader.c errmacros.h
	$(CC) $(CFLAGS) -pg fifo_reader.c
	
fifo_reader: fifo_reader.o
	$(CC) $(LFLAGS) -pg fifo_reader.o -o fifo_reader

run_fifo_reader: fifo_reader
	./fifo_reader



#############################   clean   ####################################

clean:
	rm -f *~ *.o $(EXE)


#############################   cppcheck  ####################################

cppcheck: myqueue.c myqueue.h list.c list.h tcpsocket.c tcpsocket.h storagemgr.c storagemgr.h datamgr.c datamgr.h gateway.c gateway.h common.h errmacros.h main.c
	$(CPPCHECK) --enable=all --suppress=missingIncludeSystem  myqueue.c myqueue.h list.c list.h tcpsocket.c tcpsocket.h storagemgr.c storagemgr.h datamgr.c datamgr.h gateway.c gateway.h common.h errmacros.h main.c


# list.o:list.c list.h
	# $(CC) $(LFLAGS) -fPIC -c list.c 

# liblist.so:list.o
	# $(CC) -shared -o liblist.so list.o

# copy:liblist.so 
	# sudo cp liblist.so $(LOC)

# clean:  
	# rm sensor_gateway sensor_node sensor_gateway.o tcpsocket.o sensor_db.o sensor_node.o myqueue.o
