OBJ := output.o hash.o measure.o meraki.o

libmeraki.a: $(OBJ)
	ar rcu libmeraki.a $(OBJ)
