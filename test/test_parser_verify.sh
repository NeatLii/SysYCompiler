#!/bin/bash

if [ $# -ne 1 ];
then
    echo "Usage: $0 DIRPATH"
    exit 1
fi

if [ ! -d $1 ];
then
    echo "cannot open '${1}': No such directory"
    exit 1
fi

# 目录的绝对路径
dirPath="$(realpath $1)"

# 进入项目根目录
cd $(dirname ${BASH_SOURCE[0]})/..

DRIVER="./build/test/frontend/parser_verify_tool"
if [ ! -x $DRIVER ];
then
    echo "executable file '${DRIVER}' dose not exsit, please build it"
    exit 1
fi

TEMP="./temp/info"
TEMP=$(realpath ${TEMP})

for x in $(ls $dirPath | grep -e ".sy$");
do
    absPath=${dirPath}/${x}
    echo -e "\e[34m[VERIFY]\e[0m ${absPath}"
    $DRIVER ${absPath} > ${TEMP}
    if [ $? -ne 0 ]
    then
        echo
        echo -e "\e[31;1m[FAILED]\e[0m see output in '${TEMP}'"
        exit 1
    fi
done
