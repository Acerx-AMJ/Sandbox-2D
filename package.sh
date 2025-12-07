#!/bin/bash
mkdir -p build Sandbox Sandbox/data
rm -rf build/* Sandbox/* Sandbox_Linux_x86_64.tar.gz

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cp -r assets Sandbox/
cp build/sandbox Sandbox/

# Copy Linux libraries
cp /lib/x86_64-linux-gnu/libstdc++.so.6 Sandbox/
cp /lib/x86_64-linux-gnu/libm.so.6 Sandbox/
cp /lib/x86_64-linux-gnu/libgcc_s.so.1 Sandbox/
cp /lib/x86_64-linux-gnu/libc.so.6 Sandbox/

tar -czf Sandbox_Linux_x86_64.tar.gz Sandbox/*
rm -rf Sandbox/
