all: debug
debug: makedbg finalize
release: makerel finalize
mxe: setupmxe makemxe compile
mxer: setupmxe makemxer compile

finalize: compile
	strip -s -S --strip-dwo --strip-unneeded -x -X -R .note -R .comment build/charlie
	mkdir -p bin/client bin/server bin/server/modules/linux bin/server/modules/windows bin/utils
	cp build/charlie bin/client
	-cp build/*.so  bin/server/modules/linux/
	-cp build/*.dll bin/server/modules/windows/
	cp build/cserver bin/server
	cp build/server_identity bin/server
	cp src/config/init.json bin/server/
	cp build/cutils bin/utils
	cp resources/tor/tor bin/
	cp resources/startup.bash bin/
	cp resources/setuptor.expect bin/
	cp resources/tor/hidden_service/ bin/ -r
	cp Dockerfile bin

setupmxe:
	git submodule update --init
	cd deps/mxe && make gcc pthreads boost curl libffi libltdl zlib openssl glib protobuf cpp-netlib
	sed -i '/set(CMAKE_BUILD_TYPE Release)/d' ./deps/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake
	touch setupmxe

clean:
	-rm -rf makerel makedbg proto makemxe makemxer
	-rm -rf bin

make: makedbg
makedbg: proto
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
	touch makedbg
makerel: proto
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
	touch makerel
makemxe: proto
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
	cd build && make cutils
	mv ./build/cutils cutils
	-rm -rf build
	-mkdir build
	mv ./cutils ./build/cutils
	cd build && cmake .. -DACTUAL_BUILD_TYPE=Debug -DNO_TOOLS=ON -DCMAKE_TOOLCHAIN_FILE=`pwd`/../deps/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake -DWINCC=yes
	touch makemxe
makemxer: proto
	-mkdir build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
	cd build && make cutils
	mv ./build/cutils cutils
	-rm -rf build
	-mkdir build
	mv ./cutils ./build/cutils
	cd build && cmake .. -DACTUAL_BUILD_TYPE=Release -DNO_TOOLS=ON -DCMAKE_TOOLCHAIN_FILE=`pwd`/../deps/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake -DWINCC=yes -DCNDEBUG
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
	cd ../charliebin/ && rm ./hidden_service/.gitignore && git add -A && git commit -am "$(m)" && git push dokku master
