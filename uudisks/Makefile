MAKEDEPEND = gcc -M $(CFLAGS) -o $*.d $<
include $(SRCS:.c=.P)
include uud.P uu.P
SRCS = $(wildcard *.c)
CFLAGS = `pkg-config gio-unix-2.0 --cflags`
LDFLAGS = `pkg-config gio-unix-2.0 --libs`
all: uld ulm nda
uu.c uu.h: uu.xml
	gdbus-codegen --generate-c-code=uu  --c-namespace=uu --interface-prefix=org.freedesktop uu.xml
uud.c uud.h: uud.xml
	gdbus-codegen --generate-c-code=uud  --c-namespace=uud --interface-prefix=org.freedesktop uud.xml
%.P : %.c
	$(MAKEDEPEND)
	@sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' < $*.d > $@; \
		rm -f $*.d; [ -s $@ ] || rm -f $@
%.o: %.c
	gcc -c $(CFLAGS) -o $@ $<
nda: nda.o uu.o uud.o 
	gcc nda.o uu.o uud.o $(CFLAGS) $(LDFLAGS) -o $@
uld: uld.o ludisks.o
	gcc uld.o ludisks.o $(CFLAGS) $(LDFLAGS) -o $@
ulm: ulm.o ludisks.o
	gcc ulm.o ludisks.o $(CFLASG) $(LDFLAGS) -o $@
.PHONY: clean
clean:
	-rm $(SRCS:.c=.o) $(SRCS:.c=.P) uu.c uu.h uud.c uud.h uu.P uud.P nda uld ulm
