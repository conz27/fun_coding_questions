#!/bin/bash

while [[ $# -gt 0 ]]
do
    case $1 in
        -c|--clean)
            rm -rf cmake/
            shift
            exit 0
            ;;
    esac
done

mkdir -p cmake
cd cmake && cmake -G "Unix Makefiles" ../ && make
