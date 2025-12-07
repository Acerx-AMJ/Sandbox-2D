#!/bin/bash
mkdir build
rm -rf build/*

cmake -B build
cmake --build build
