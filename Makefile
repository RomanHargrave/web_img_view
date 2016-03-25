CC      	= gcc
CGTK		= $(shell pkg-config --libs --cflags gtk+-2.0)
CWAND 		= $(shell pkg-config --libs --cflags MagickWand)
CCLDFLAGS	= -Ofast -ffast-math -lX11 -lm -lrt --std=c99 $(CWAND) $(CGTK)
CFLAGS  	= $(CCLDFLAGS) -I./include 
STRIP   	= sstrip

.PHONY: bin/webim_main

bin:
	-mkdir bin

bin/webim_main.o: src/webim_main.c bin
	$(CC) $(CFLAGS) $(GTK) -c $< -o $@

bin/webim_display.o: src/webim_display.c include/webim/display.h bin
	$(CC) $(CFLAGS) $(GTK) -c $< -o $@

bin/webim_defish.o: src/webim_defish.c bin
	$(CC) $(CFLAGS) $(CWAND) -c $< -o $@

bin/webim_magick_tools.o: src/webim_magick_tools.c bin
	$(CC) $(CFLAGS) -c $< -o $@

bin/webim_main: bin/webim_defish.o bin/webim_magick_tools.o bin/webim_display.o bin/webim_main.o
	$(CC) $(CCLDFLAGS) $(GTK) $? -o $@

webim_defish_test: src/webim_defish_test.c bin/webim_defish.o 
	$(CC) $(CFLAGS) $(CWAND) $^ -o $@

