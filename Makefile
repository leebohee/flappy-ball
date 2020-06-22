CC = ~/buildroot/output/host/bin/arm-linux-gcc
TARGET = flappy_ball

all: $(TARGET)

$(TARGET): display.o ui.o gpio.o flappy_ball.c
	$(CC) display.o ui.o gpio.o flappy_ball.c -o flappy_ball

display.o: display.h display.c font.h
	$(CC) -c -o display.o display.c

ui.o: ui.h ui.c
	$(CC) -c -o ui.o ui.c 

gpio.o: gpio.h gpio.c
	$(CC) -c -o gpio.o gpio.c

clean:
	rm -f *.o
	rm -f $(TARGET)