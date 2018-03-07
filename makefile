all: myls

myls: main.o myls.o print.o util.o ./err_handlers/err_handlers.o ./path_alloc/path_alloc.o
	gcc -Wall main.o myls.o print.o util.o ./err_handlers/err_handlers.o ./path_alloc/path_alloc.o -o myls

main.o: main.c myls.h ./err_handlers/err_handlers.h
	gcc -Wall -c main.c

myls.o: myls.c myls.h ./err_handlers/err_handlers.h ./path_alloc/path_alloc.h
	gcc -Wall -c myls.c

print.o: print.c myls.h ./err_handlers/err_handlers.h
	gcc -Wall -c print.c

util.o: util.c myls.h
	gcc -Wall -c util.c

err_handlers.o: ./err_handlers/err_handlers.c
	gcc -Wall -c ./err_handlers/err_handlers.c

path_alloc.o: ./path_alloc/path_alloc.c
	gcc -Wall -c ./path_alloc/path_alloc.c

.PHONY: clean
clean:
	rm -rf *.o ./err_handlers/*.o ./path_alloc/*.o
