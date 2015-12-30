all: debug
debug: makessl makedeps maketor makeboost makeprotolib makedbg finalize
release: makessl makedeps maketor makeboost makeprotolib makerel strip finalize
mxe: setupmxe makemxe compile
mxer: setupmxe makemxer compile

.PHONY: strip finalize clean dclean drun dbash dcleanall push compile valgrind run server client
BOOST_COMPILE_ARGS=--layout=system cxxflags="-std=c++11" linkflags="-std=c++11" link=static threading=multi runtime-link=static --without-python -q --without-wave --without-container --without-graph --without-graph_parallel --without-locale --without-mpi --without-context --without-coroutine

protoc="../../deps/protobuf/final/bin/protoc"

strip: compile
	strip -s -S --strip-dwo --strip-unneeded -x -X -R .note -R .comment build/charlie

finalize: compile
	mkdir -p bin/client bin/server bin/server/modules bin/server/server_modules bin/utils
	-cp build/charlie bin/client/charlie
	-cp build/*.so  bin/server/modules
	-cp build/*.dll bin/server/modules
	-cp build/cserver bin/server
	-cp build/server_modules/* bin/server/server_modules
	cp build/server_identity bin/server
	cp src/config/init.json bin/server/
	cp build/cutils bin/utils
	cp resources/tor/tor bin/
	cp resources/startup.bash bin/
	cp resources/docker/charlie/Dockerfile bin

.setupmxe:
	cd deps/mxe && make gcc pthreads boost curl libffi libltdl zlib openssl glib protobuf
	sed -i '/set(CMAKE_BUILD_TYPE Release)/d' ./deps/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake
	touch .setupmxe
setupmxe: .setupmxe

clean:
	#-rm -rf .makerel .makedbg .proto
	-rm -rf .proto
	-rm -rf bin

dclean: clean
	-rm -rf makeboost makessl

.makessl:
	-cd ./deps/openssl && rm -rf ./final/ && make clean && make dclean && mkdir ./final/
	cd ./deps/openssl && ./config --prefix="`pwd`/final/" --openssldir="`pwd`/final/" -fPIC -DOPENSSL_PIC -D__FILE__="\"\"" -D__DIR__="\"\"" -Wno-builtin-macro-redefined && make && make install
	sed -i -e "s/^my \$$dir.*$$/my \$$dir = \"\";/g" ./deps/openssl/final/bin/c_rehash
	sed -i -e "s/^my \$$prefix.*$$/my \$$prefix = \"\";/g" ./deps/openssl/final/bin/c_rehash
	# Small hack, just comment out all the find_package in curl
	sed -i -e "s/ find_package/ #find_package/g" ./deps/curl/CMakeLists.txt
	touch .makessl
makessl: .makessl

.makedeps:
	cd deps && mkdir -p build final
	sed -i -e "s/ find_package/ #find_package/g" ./deps/libevent/CMakeLists.txt
	sed -i -e '/-fPIE/d' ./deps/libevent/CMakeLists.txt
	sed -i -e '/CURL_SOURCE_DIR}\/curl-config.in/,+7d' ./deps/curl/CMakeLists.txt
	sed -i -e '/CURL_SOURCE_DIR}\/libcurl.pc.in/,+4d' ./deps/curl/CMakeLists.txt
	sed -i -e '/auth using \%s with user/,+4d' ./deps/curl/lib/http.c
	cd deps/build && cmake .. -DCMAKE_BUILD_TYPE=Release
	cd deps/build && make -j4 && make install
	touch .makedeps
makedeps: .makedeps

.maketor:
	if [ ! -f ./deps/tor/.charlie_patch1_applied ]; then cd deps/tor/ && git am --no-signed < ../patch/charlie_tor_patch1.patch; fi
	cd deps/tor/ && export CFLAGS="-fPIC" && ./autogen.sh && ./configure --disable-asciidoc
	cd deps/tor/ && sed -i -e "s/-fPIE //g" Makefile
	cd deps/tor/ && make -j4
	touch .maketor
maketor: .maketor

.makeboost:
	# Hack to enable FPIC
	sed -i -e "s# \= shared# \= static#g" ./deps/boost/tools/build/src/tools/gcc.jam
	cd ./deps/boost/ && ./bootstrap.sh --prefix="`pwd`/final/"
	cd ./deps/boost/ && ./b2 install $(BOOST_COMPILE_ARGS)
	cd ./deps/boost/final/include/boost/iostreams/ && sed '/typeid/d' -i detail/streambuf/indirect_streambuf.hpp && sed '/typeid/d' -i detail/streambuf/direct_streambuf.hpp
	find ./deps/process/boost/process/ -type f -name '*.hpp' -exec sed -i -e "s/Windows.h/windows.h/g" -e "s/Shellapi.h/shellapi.h/g" {} \;
	touch .makeboost
makeboost: .makeboost

.makeprotolib:
	-cd ./deps/protobuf && make clean
	cd ./deps/protobuf/ && ./autogen.sh && ./configure --with-pic --prefix=`pwd`/final && make -j4 && make -j4 install
	touch .makeprotolib
makeprotolib: .makeprotolib

make: makedbg
.makedbg: proto
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
	touch .makedbg
makedbg: .makedbg
.makerel: proto
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
	touch .makerel
makerel: .makerel
.makemxe: proto
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
	cd build && make cutils
	mv ./build/cutils cutils
	-rm -rf build
	-mkdir build
	mv ./cutils ./build/cutils
	cd build && cmake .. -DACTUAL_BUILD_TYPE=Debug -DNO_TOOLS=ON -DCMAKE_TOOLCHAIN_FILE=`pwd`/../deps/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake -DWINCC=yes
	touch .makemxe
makemxe: .makemxe
.makemxer: proto
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
	cd build && make cutils
	mv ./build/cutils cutils
	-rm -rf build
	-mkdir build
	mv ./cutils ./build/cutils
	cd build && cmake .. -DACTUAL_BUILD_TYPE=Release -DNO_TOOLS=ON -DCMAKE_TOOLCHAIN_FILE=`pwd`/../deps/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake -DWINCC=yes -DCNDEBUG
	touch .makemxer
makemxer: .makemxer
compile:
	cd build && make -j4
run: all
	cd build && ./charlie
.proto:
	-rm -rf ./include/protogen/ ./src/protogen/ ./src/server_protogen/
	-mkdir ./src/protogen/
	-mkdir ./include/protogen/
	cd src/proto && $(protoc) --cpp_out=../protogen/ ./*.proto
	cp ./src/protogen/*.h ./include/protogen/
	touch .proto
proto: .proto

valgrind: makedbg compile
	cd bin/client && valgrind --leak-check=full --show-reachable=yes --track-origins=yes --suppressions=../../valgrind.supp ./charlie

dimage: debug finalize
	sudo docker build -t charlie/cserver ./bin/

drun:
	-sudo docker rm -f cserver
	sudo docker run -dt --name cserver charlie/cserver

dbash:
	sudo docker exec -it cserver /bin/bash

dcleanall:
	sudo docker rm -f `sudo docker ps --no-trunc -aq`

docker: release finalize
	cp Dockerfile bin/
	cd bin && docker build -t "paralin/charlie:v1" .

server: all
	cd ./bin/server/ && ./cserver

client: all
	cd ./bin/client/ && ./charlie

# Persist clean
pc:
	-rm ~/.kde/share/autostart/*
	-rm ~/.config/autostart/*
	-rm -rf ~/.dbus/system/

.torhost_image:
	cp ./resources/setuptor.expect ./resources/docker/torhost/
	cd resources/docker/torhost && ./build.bash
	touch .torhost_image
torhost_image: .torhost_image

.charlie_image: finalize
	cd bin && bash ../resources/docker/charlie/build.bash
	touch .charlie_image
charlie_image: .charlie_image
