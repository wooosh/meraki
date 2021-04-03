OBJ := output.o hash.o input.o measure.o meraki.o

libmeraki.a: $(OBJ)
	ar rcu libmeraki.a $(OBJ)
