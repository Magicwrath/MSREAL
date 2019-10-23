#Hello make
sources=$(wildcard *.c)
obj=$(sources:.c=.o) #ne kucaj razmake ako boga znas

all:customize
customize : $(obj)
	gcc -o $@ $^ #@ je da bi kopirao ime hello (odnosno gde je upisana komanda) a ^ je da se odnosi na fajlove navedene u hello
%.o : %.c
	gcc -c $^
%.d : %.c
	gcc -MM $^ > $@;
.PHONY : clean
clean : 
	rm -f hello *.o *.~

-include $(sources:.c=.d) #ovo pokrece na pocetku! skontace da nema d fajlova, i prolazi kroz kod do pravila za pravljenje d fajlova
