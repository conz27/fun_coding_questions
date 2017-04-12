#!/bin/bash

for arg in "$@"
do
    case $arg in
        -c|--clean)
            echo "Cleaning all build artifacts..."
            rm -rf cmake/
            echo "Success!"
            exit 0
            ;;
    esac
done

mkdir -p cmake
cd cmake && cmake -G "Unix Makefiles" ../ && make
