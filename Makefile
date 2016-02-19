CC      	= gcc
CGTK		= $(shell pkg-config --libs --cflags gtk+-2.0)
CWAND 		= $(shell pkg-config --libs --cflags MagickWand)
CCLDFLAGS	= -lX11 -lm $(CGTK) $(CWAND)
CFLAGS  	= $(CCLDFLAGS) -I./include 
STRIP   	= sstrip

.PHONY: all

all: webim_main webim_defish_test

bin:
	-mkdir bin

bin/webim_main.o: src/webim_main.c bin
	$(CC) $(CFLAGS) -c $< -o $@

bin/webim_defish.o: src/webim_defish.c bin
	$(CC) $(CFLAGS) $(CWAND) -c $< -o $@

webim_main: bin/*.o
	$(CC) $(CCLDFLAGS) $? -o $@

webim_defish_test: src/webim_defish_test.c bin/webim_defish.o 
	$(CC) $(CFLAGS) $(CWAND) $^ -o $@

