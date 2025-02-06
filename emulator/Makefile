.PHONY: all clean test

all:
	mkdir -p obj
	g++ -o obj/emulator emulator.cpp -g -std=c++11

clean:
	rm -rf obj

test:
	./obj/emulator test.txt
