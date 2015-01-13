#!/bin/bash
rm ./include/proto/*
rm ./src/proto/*
./resources/protoc/protoc -I=./resources/protobufs --cpp_out=./src/proto ./resources/protobufs/charlie.proto
./resources/protoc/protoc -I=./resources/protobufs --cpp_out=./src/server_proto ./resources/protobufs/charlie_server.proto
cp ./src/proto/*.h ./include/proto
cp ./src/server_proto/*.h ./include/server_proto
