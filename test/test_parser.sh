#!/bin/bash
# test if flex and bison can build ast

if [ $# -ne 1 ]; then
    echo "Usage: $0 DIRPATH"
    exit 1
fi

if [ ! -d $1 ]; then
    echo "cannot open '${1}': No such directory"
    exit 1
fi

# 目录的绝对路径
dirPath="$(realpath $1)"

# 进入项目根目录
cd $(dirname ${BASH_SOURCE[0]})/..

DRIVER="./build/src/frontend/parser_tool"
if [ ! -x $DRIVER ]; then
    echo "executable file '${DRIVER}' dose not exsit, please build it"
    exit 1
fi

if [ ! -d "./tmp" ]; then
    mkdir "./tmp"
fi
if [ ! -d "./tmp/ast" ]; then
    mkdir "./tmp/ast"
fi
TEMP="./tmp/ast"
TEMP=$(realpath ${TEMP})

for x in $(ls $dirPath | grep -e ".sy$");
do
    srcPath=${dirPath}/${x}
    desPath=${TEMP}/${x%.*}.ast
    echo -e "\e[34m[BUILD]\e[0m from \e[33m${srcPath}\e[0m to \e[33m${desPath}\e[0m"
    $DRIVER ${srcPath} > ${desPath}
    if [ $? -ne 0 ]; then
        echo
        echo -e "\e[31;1m[FAILED]\e[0m see output in '${desPath}'"
    fi
done

echo -e "\e[32;1m[SUCCESS]\e[0m"
