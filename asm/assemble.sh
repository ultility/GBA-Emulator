clean=clean
BASEDIR=$(dirname $0)
compile_args=""
build_args="-march=armv4t -mcpu=arm7tdmi"
link_args=""

echo "input file: $1"

if [ "$1" = "$clean" ]
then
    echo "cleaning"
    rm ${BASEDIR}/*.o ${BASEDIR}/*.elf
else
    echo "assembling"
    arm-linux-gnueabi-as ${build_args} ${compile_args} ${BASEDIR}/$1.s -o ${BASEDIR}/$1.o
    arm-linux-gnueabi-gcc-14 ${build_args} ${link_args} ${BASEDIR}/$1.o -o ${BASEDIR}/$1.elf -nostdlib
fi
