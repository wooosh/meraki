CFLAGS += -Iinclude -fPIC
OBJ := output.o input.o measure.o term.o

libmeraki.a: $(OBJ)
	ar rcu libmeraki.a $(OBJ)

libmeraki.so: $(OBJ)
	cc -shared -o libmeraki.so $(OBJ)

.PHONY: clean
clean:
	-rm $(OBJ) libmeraki.a libmeraki.so
