#!/bin/bash

while [[ $# -gt 1 ]]
do
    case $1 in
        -c|--clean)
            rm -rf cmake/
            exit 0
            ;;
    esac
done

mkdir -p cmake
cd cmake && cmake -G "Unix Makefiles" ../ && make
