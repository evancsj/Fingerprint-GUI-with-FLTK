gui:gui.o UART.o
	g++ -I/usr/local/include -I/usr/include/freetype2 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT -o gui $^  /usr/local/lib/libfltk.a -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lpthread -ldl -lm -lX11
gui.o:gui.cpp UART.h
	g++ -c $<
UART.o:UART.cpp UART.h
	g++ -c $<
clean:
	rm gui gui.o UART.o
