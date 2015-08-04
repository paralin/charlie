all: debug
debug: makessl makeboost makeprotolib makedbg finalize
release: makessl makeboost makeprotolib makerel strip finalize
mxe: setupmxe makemxe compile
mxer: setupmxe makemxer compile

.PHONY: strip finalize clean dclean drun dbash dcleanall push compile valgrind run

protoc="../../deps/protobuf/final/bin/protoc"

strip: compile
	strip -s -S --strip-dwo --strip-unneeded -x -X -R .note -R .comment build/charlie

finalize: compile
	mkdir -p bin/client bin/server bin/server/modules/linux bin/server/modules/windows bin/utils
	-cp build/charlie bin/client/charlie
	-cp build/*.so  bin/server/modules/linux/
	-cp build/*.dll bin/server/modules/windows/
	-cp build/cserver bin/server
	cp build/server_identity bin/server
	cp src/config/init.json bin/server/
	cp build/cutils bin/utils
	cp resources/tor/tor bin/
	cp resources/startup.bash bin/
	cp resources/setuptor.expect bin/
	cp resources/tor/hidden_service/ bin/ -r
	cp Dockerfile bin

.setupmxe:
	git submodule update --init
	cd deps/mxe && make gcc pthreads boost curl libffi libltdl zlib openssl glib protobuf
	sed -i '/set(CMAKE_BUILD_TYPE Release)/d' ./deps/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake
	touch .setupmxe
setupmxe: .setupmxe

clean:
	-rm -rf makerel makedbg proto makemxe makemxer
	-rm -rf bin

dclean: clean
	-rm -rf makeboost makessl

.makessl:
	git submodule update --init
	-cd ./deps/openssl && rm -rf ./final/ && make clean && make dclean && mkdir ./final/
	cd ./deps/openssl && export CFLAGS="-fPIC" && ./config --prefix="`pwd`/final/" -fPIC -DOPENSSL_PIC && make -j4 && make install
	touch .makessl
makessl: .makessl

.makeboost:
	git submodule update --init && cd ./deps/boost/ && git submodule update --init
	# Hack to enable FPIC
	sed -e "# = shared# = static#g" -i ./deps/boost/tools/build/src/tools/gcc.jam
	cd ./deps/boost/ && ./bootstrap.sh --prefix="`pwd`/final/" && rm -rf ./final && mkdir -p final/include/boost
	cd ./deps/boost/ && ./b2 headers install cxxflags="-std=c++11" linkflags="-std=c++11" variant=release link=static threading=multi runtime-link=static --without-python --layout=system -q --without-wave --without-container --without-graph --without-graph_parallel --without-locale --without-mpi --without-context --without-coroutine #-d0
	cd ./deps/boost/ && cp libs/scope_exit/include/boost/scope_exit.hpp final/include/boost/ && cp libs/utility/include/boost/utility/string_ref.hpp final/include/boost/utility/ && cp -r libs/exception/include/boost/exception/* final/include/boost/exception/
	cd ./deps/boost/ && cp boost/*.hpp final/include/boost/
	cd ./deps/boost/ && cp boost/utility/*.hpp final/include/boost/utility/
	cd ./deps/boost/ && cp -r libs/logic/include/boost/logic/ final/include/boost/
	cd ./deps/boost/ && cp -r libs/assign/include/boost/assign/ final/include/boost/
	cd ./deps/boost/final/include/boost/iostreams/ && sed '/typeid/d' -i detail/streambuf/indirect_streambuf.hpp && sed '/typeid/d' -i detail/streambuf/direct_streambuf.hpp
	touch .makeboost
makeboost: .makeboost

.makeprotolib:
	git submodule update --init && cd ./deps/protobuf/ && git submodule update --init
	-cd ./deps/protobuf && make clean
	cd ./deps/protobuf/ && ./autogen.sh && ./configure --with-pic --prefix=`pwd`/final && make -j4 && make -j4 install
	touch makeprotolib
makeprotolib: .makeprotolib

make: makedbg
.makedbg: proto
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
	touch makedbg
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
	cd build && valgrind --leak-check=full --show-reachable=yes --track-origins=yes --suppressions=../valgrind.supp ./charlie

dimage: debug finalize
	sudo docker build -t charlie/cserver ./bin/

drun:
	-sudo docker rm -f cserver
	sudo docker run -dt --name cserver charlie/cserver

dbash:
	sudo docker exec -it cserver /bin/bash

dcleanall:
	sudo docker rm -f `sudo docker ps --no-trunc -aq`

push: finalize
	@if [ ! -d "../charliebin/" ]; then echo "Charlie binary repository does not exist." && exit 5; fi
	rsync -rav --exclude='.git/' --exclude="client/" --delete bin/ ../charliebin/
	cd ../charliebin/ && git add -A && git commit -am "$(m)" && git push dokku master
