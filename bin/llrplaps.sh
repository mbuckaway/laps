#!/bin/bash
#
# Script to run llrplaps and resolve run-time dependencies.
#

llrplapspath=/home/icunning/laps

# set default directory to llrplaps bin directory

cd $llrplapspath/bin;

if [ ! -x llrplaps ]
  then
    chmod +x llrplaps;
fi

exec ./llrplaps
