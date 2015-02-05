all: proto makedbg compile
release: proto makerel compile
mxe: setupmxe proto makemxe compile
mxer: setupmxe proto makemxer compile

setupmxe:
	git submodule update --init
	cd deps/mxe && make gcc boost curl libffi libltdl zlib openssl glib protobuf
	touch setupmxe

clean:
	-rm -rf build makerel makedbg proto makemxe makemxer

make: makedbg
makedbg:
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=DEBUG
	touch makedbg
makerel:
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=RELEASE
	touch makerel
makemxe:
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_TOOLCHAIN_FILE=`pwd`/../deps/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake -DWINCC=yes
	touch makemxe
makemxer:
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_TOOLCHAIN_FILE=`pwd`/../deps/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake -DWINCC=yes
	touch makemxer
compile:
	cd build && make -j4
run: all
	cd build && ./charlie
proto:
	-rm -rf ./include/protogen/ ./src/protogen/ ./src/server_protogen/
	-mkdir ./src/protogen/
	-mkdir ./include/protogen/
	cd src/proto && protoc --cpp_out=../protogen/ ./*.proto
	cp ./src/protogen/*.h ./include/protogen/
	touch proto
