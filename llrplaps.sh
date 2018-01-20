#!/bin/bash
#
# Script to run llrplaps and resolve run-time dependencies.
#

llrplapspath=/home/icunning/laps


cd $llrplapspath;

#if [ ! -x $llrplapspath/bin/llrplaps ]
#then { chmod +x $llrplapspath/bin/llrplaps; }

exec ./bin/llrplaps
