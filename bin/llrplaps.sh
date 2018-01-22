#!/bin/bash
#
# Script to run llrplaps and resolve run-time dependencies.
#

llrplapspath=/home/icunning/laps/bin

# set default directory to llrplaps bin directory

cd $llrplapspath;

if [ ! -x llrplaps ]
  then
    chmod +x llrplaps;
fi

exec ./llrplaps
