#!/bin/sh
if [ -z "$CC" ];
then
	CC=cc
fi
SYSNAME=$(uname)
case $SYSNAME in
	*indow*)
	C_OPT="-lraylib -lgdi32 -lwinmm"
	;;
	*inux*)
	C_OPT="-lraylib -lGL -lm -ldl -lrt -lpthread"
	;;
	*)
	;;
esac	
mkdir -p bin
COMPILE="$CC ./src/*.c $C_OPT -o ./bin/fontgen"
echo $COMPILE
$COMPILE
