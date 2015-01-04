all: make compile

clean:
	-rm -rf build make

make:
	-mkdir build
	cd build && cmake ..
	touch make
compile:
	cd build && make -j4
run: cmake compile
	cd build && ./charlie
