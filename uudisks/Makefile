all: uld ulm nda
MAKEDEPEND = gcc -M $(CFLAGS) -o $*.d $<
include $(SRCS:.c=.P)
include uud.P uu.P
SRCS = $(wildcard *.c)
CFLAGS = `pkg-config gio-unix-2.0 --cflags` -O2
LDFLAGS = `pkg-config gio-unix-2.0 --libs`
COMMON_OBJ = uu.o uud.o
uu.c uu.h:
	gdbus-codegen --generate-c-code=uu  --c-namespace=uu --interface-prefix=org.freedesktop /usr/share/dbus-1/interfaces/org.freedesktop.UDisks.xml
uud.c uud.h:
	gdbus-codegen --generate-c-code=uud  --c-namespace=uud --interface-prefix=org.freedesktop /usr/share/dbus-1/interfaces/org.freedesktop.UDisks.Device.xml
%.P : %.c
	$(MAKEDEPEND)
	@sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' < $*.d > $@; \
		rm -f $*.d; [ -s $@ ] || rm -f $@
%.o: %.c
	gcc -c $(CFLAGS) -o $@ $<
nda uld ulm: $(COMMON_OBJ)
nda: nda.o
	gcc nda.o $(COMMON_OBJ) $(CFLAGS) $(LDFLAGS) -o $@
uld: uld.o
	gcc uld.o $(COMMON_OBJ) $(CFLAGS) $(LDFLAGS) -o $@
ulm: ulm.o
	gcc ulm.o $(COMMON_OBJ) $(CFLASG) $(LDFLAGS) -o $@
.PHONY: clean
clean:
	-rm $(SRCS:.c=.o) $(SRCS:.c=.P) uu.c uu.h uud.c uud.h uu.P uud.P nda uld ulm
