#!/bin/sh
appname=`basename $0 | sed s,\.sh$,,`

echo $appname

dirname=`dirname $0`
tmp="${dirname#?}"


if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi


LD_LIBRARY_PATH=$dirname../lib
export LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH

$dirname/$appname "$@"