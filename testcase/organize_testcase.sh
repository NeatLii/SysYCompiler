if [ $# -ne 3 ]
then
    echo "need three parameter: $0 srcFile desDir desName"
    exit 1
fi

if [ ! -f $1 ]
then
    echo "no such file: $1"
    exit 1
fi

if [ ! -d $2 ]
then
    echo "no such dir: $2"
    exit 1
fi

srcDir=$(realpath $(dirname $1))
var=$(basename $1)
srcName=${var%.*}

desDir=$(realpath $2)
desName=$3

if [ -f ${srcDir}/${srcName}.sy ]
then
    cp ${srcDir}/${srcName}.sy ${desDir}/sy/${desName}.sy
    sed -i 's/\r//' ${desDir}/sy/${desName}.sy
    echo "cp ${srcDir}/${srcName}.sy to ${desDir}/sy/${desName}.sy"
fi

if [ -f ${srcDir}/${srcName}.in ]
then
    cp ${srcDir}/${srcName}.in ${desDir}/in/${desName}.in
    sed -i 's/\r//' ${desDir}/in/${desName}.in
    echo "cp ${srcDir}/${srcName}.in to ${desDir}/in/${desName}.in"
fi

if [ -f ${srcDir}/${srcName}.out ]
then
    cp ${srcDir}/${srcName}.out ${desDir}/out/${desName}.out
    sed -i 's/\r//' ${desDir}/out/${desName}.out
    echo "cp ${srcDir}/${srcName}.out to ${desDir}/out/${desName}.out"
fi
