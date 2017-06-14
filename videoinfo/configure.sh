#!/bin/bash

BUILD_DIR=build

mkdir -p $BUILD_DIR

cd $BUILD_DIR && cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../
