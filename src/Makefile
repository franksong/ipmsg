CC = clang
CFLAGS = -g
objects = main.o init.o user_host.o common.o send_receive.o ipmsg_thread.o

ipmsg: $(objects)
	$(CC) $(CFLAGS) -o ipmsg $(objects) -lpthread

main.o: main.c init.h ipmsg_thread.h common.h
	$(CC) $(CFLAGS) -c $^ 

init.o: init.c common.h user_host.h ipmsg.h init.h
	$(CC) $(CFLAGS) -c $^

user_host.o: user_host.c user_host.h common.h
	$(CC) $(CFLAGS) -c $^

common.o: common.c common.h ipmsg.h
	$(CC) $(CFLAGS) -c $^

send_receive.o: send_receive.c send_receive.h init.h common.h ipmsg.h user_host.h
	$(CC) $(CFLAGS) -c $^

ipmsg_thread.o: ipmsg_thread.c ipmsg_thread.h init.h send_receive.h common.h ipmsg.h
	$(CC) $(CFLAGS) -c $^

.PHONY: clean
clean:
	rm -f ipmsg $(objects) *.gch
cgch:
	rm -f *.gch
