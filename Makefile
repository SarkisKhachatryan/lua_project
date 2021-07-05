all: 
	mkdir build
	cd build; cmake ..
	cd build/main; make
	./build/main/LuaTutorial
	rm -rf build

build:
	mkdir build
	cd build; cmake ..

run:
	cd build/main; make
	./build/main/LuaTutorial

build_and_run: build run

clean:
	rm -rf build