#!/bin/sh

echo  "---ready to push---"

if [ "$#" -lt "1" ]; then
    echo "usage: $0 <area> <hours>"
	    exit 2
   else
       git push $1 master:master
   fi
echo "====== push end ======"
