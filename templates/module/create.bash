#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT=../..
CUTILS=$PROJECT_ROOT/bin/utils/cutils

if [ ! -f $CUTILS ]; then
  echo "At least cutils must be built."
  exit 1
fi

MODULE_NAME=$(echo $1 | awk '{print tolower($0)}')
if [ -z "$MODULE_NAME" ]; then
  echo "Creates a client module."
  echo "Usage: ./create.bash modulename"
  exit 1
fi

# Upper case first letter
MODULE_NAME_UC="$(echo "$MODULE_NAME" | sed 's/.*/\u&/')"

# Get the hash
MODULE_ID=$($CUTILS hashstr --str $MODULE_NAME)

echo "Module name: $MODULE_NAME"
echo "Proper name: $MODULE_NAME_UC"
echo "ID:          $MODULE_ID"

MODULE_PATH=$PROJECT_ROOT/src/modules/$MODULE_NAME
MODULE_HEADER_PATH=$PROJECT_ROOT/include/modules/$MODULE_NAME
MODULE_PROTO_PATH=$PROJECT_ROOT/src/proto

if [ -d $MODULE_PATH ]; then
  echo "Module path ($MODULE_PATH) already exists!"
  exit 1
fi

read -r -p "Create? [Y/n] " response
response=${response,,} # tolower
if [[ $response =~ ^(yes|y| ) ]] || [ -z $response ]; then
  echo "Creating..."
else
  exit 1
fi

mkdir -p $MODULE_PATH
cp $DIR/module.cpp $MODULE_PATH/${MODULE_NAME}.cpp
sed \
  -e "s;{MODULE_NAME};$MODULE_NAME;g" \
  -e "s;{MODULE_NAME_UC};$MODULE_NAME_UC;g" \
  -e "s;{MODULE_ID};$MODULE_ID;g" \
  $DIR/module.cpp > $MODULE_PATH/${MODULE_NAME}.cpp

mkdir -p $MODULE_HEADER_PATH
sed \
  -e "s;{MODULE_NAME};$MODULE_NAME;g" \
  -e "s;{MODULE_NAME_UC};$MODULE_NAME_UC;g" \
  -e "s;{MODULE_ID};$MODULE_ID;g" \
  $DIR/Module.h > $MODULE_HEADER_PATH/${MODULE_NAME_UC}.h

sed \
  -e "s;{MODULE_NAME};$MODULE_NAME;g" \
  -e "s;{MODULE_NAME_UC};$MODULE_NAME_UC;g" \
  -e "s;{MODULE_ID};$MODULE_ID;g" \
  $DIR/module.cmake > $MODULE_PATH/module.cmake

cp $DIR/module.proto $MODULE_PROTO_PATH/${MODULE_NAME}.proto
