#Hello make

sources=$(wildcard *.c)
obj=$(sources:.c=.o)

all:hello
hello : $(obj)
	gcc -o $@ $^ #@ je da bi kopirao ime hello (odnosno gde je upisana komanda) a ^ je da se odnosi na fajlove navedene u hello
%.o : %.c
	gcc -c $^
.PHONY : clean
clean : 
	rm -f hello *.o *.~
