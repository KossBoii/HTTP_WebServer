objects = diamondhttp.o server.o data_utils.o parser.o

httpforum: $(objects)
	g++ -o ./../httpserver $(objects) -lpthread -ljsoncpp

diamondhttp.o: diamondhttp.h server.h parser.h

server.o: server.h diamondhttp.h parser.h

parser.o: parser.h diamondhttp.h data_utils.h

data_utils.o: data_utils.h parser.h diamondhttp.h user.h question.h answer.h helper_funcs.h


.PHONY: clean
clean:
	rm -f $(objects) *.o
