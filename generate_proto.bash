#!/bin/bash
rm ./include/proto/*
rm ./src/proto/*
./resources/protoc/protoc -I=./resources/protobufs --cpp_out=./src/proto ./resources/protobufs/charlie.proto
cp ./src/proto/*.h ./include/proto
