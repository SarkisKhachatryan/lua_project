all: build_and_run clean

build:
	mkdir build
	cd build; cmake ..

run:
	cd build/main; make
	./build/main/LuaTutorial

build_and_run: build run

clean:
	rm -rf build