CC=gcc
CFLAGS= -lresistance -lpower -lcomponent `pkg-config --cflags --libs gtk+-2.0 libgnome-2.0 libgnomeui-2.0`
PREFIX = /usr
INSTDIR = $(PREFIX)/bin

make: main.c
	$(CC) main.c -o elektrotestgtk $(CFLAGS)

install: main.c
	install -d '$(INSTDIR)'
	install -t '$(INSTDIR)' elektrotestgtk

uninstall:
	-rm $(INSTDIR)/elektrotestgtk

clean:
	-rm elektrotestgtk
