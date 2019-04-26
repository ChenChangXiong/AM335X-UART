#!/bin/sh

echo  "---ready to push---"

if [ "$#" -ne "2" ]; then
    echo "usage: $0 <area> <hours>"
	    exit 2
   fi
git push $1 msater:master
    echo "====== push end ======"
