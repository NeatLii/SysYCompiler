#!/bin/bash

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

DRIVER="./build/src/frontend/ast_to_ir_tool"
if [ ! -x $DRIVER ]; then
    echo "executable file '${DRIVER}' dose not exsit, please build it"
    exit 1
fi
DRIVER=$(realpath $DRIVER)

LIB="./build/lib/libsysy/libsysy.a"
if [ ! -f $LIB ]; then
    echo "static lib file '${LIB}' dose not exsit, please build it"
    exit 1
fi
LIB=$(realpath $LIB)

if [ ! -d "./tmp" ]; then
    mkdir "./tmp"
fi
if [ ! -d "./tmp/ll/" ]; then
    mkdir "./tmp/ll/"
fi
if [ ! -d "./tmp/elf/" ]; then
    mkdir "./tmp/elf/"
fi
TMP="./tmp/"
TMP=$(realpath $TMP)

ANS=$(cd ${dirPath}/../out; pwd)
IN=$(cd ${dirPath}/../in; pwd)

for x in $(ls $dirPath | grep -e ".sy$");
do
    name=${x%.*}
    srcPath=${dirPath}/${x}
    irPath=${TMP}/ll/${name}.ll
    elfPath=${TMP}/elf/${name}
    outPath=${TMP}/out/${name}.out
    ansPath=${ANS}/${name}.out
    inPath=${IN}/${name}.in

    echo -e "\e[34m[COMPILE]\e[0m \e[33m${srcPath}\e[0m"

    $DRIVER ${srcPath} > ${irPath}
    if [ $? -ne 0 ]; then
        echo
        echo -e "\e[31;1m[FAILED]\e[0m generate ir, see output in '${irPath}'"
        exit 1
    fi

    clang ${irPath} ${LIB} -o ${elfPath}
    if [ $? -ne 0 ]; then
        echo
        echo -e "\e[31;1m[FAILED]\e[0m generate elf, see output in '${elfPath}'"
        exit 1
    fi

    if [ -f ${inPath} ]; then
        ${elfPath} > ${outPath} < ${inPath}
    else
        ${elfPath} > ${outPath}
    fi
    result=$?
    # 若输出结果的文件不以换行符结尾，且不为空，则添加换行符
    if [ $(tail -n1 ${outPath} | wc -l) -eq 0 ] && [ $(cat ${outPath} | wc -c) -ne 0 ]; then
        echo >> ${outPath}
    fi
    echo $result >> ${outPath}
    # 比较输出结果
    diff -b ${outPath} ${ansPath}
    if [ $? -ne 0 ]; then
        echo
        echo -e "\e[31;1m[FAILED]\e[0m wrong result, see answer in '${ansPath}'"
        exit 1
    fi
done

echo -e "\e[32;1m[SUCCESS]\e[0m"
