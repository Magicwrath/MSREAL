sources=$(wildcard *.c)
objs=$(sources:.c=.o)

all: application
application: $(objs)
	@echo "Building output binary.."
	$(CC) -o  $@ $(objs) -lm
%.o: %.c
	@echo "Compiling sources.."
	$(CC) -c $(sources)
%.d: %.c
	@echo -n  "Creating dependancies.."
	@echo $@
	@set -e; rm -f $@; \
	$(CC) -MM -lm $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,    \($*\)\.o[ :]*     ,    \1.o $@ :   ,   g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
clean:
	@rm -f application *.o *.d*
	@echo "Clean done.."

-include $(sources:.c=.d)

