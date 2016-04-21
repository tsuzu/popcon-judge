#! /bin/bash

g++ -std=c++14 -O2 -o ./bin/popj ./src/main.cpp /usr/lib/libsioclient.a -lboost_system -lboost_program_options -lboost_filesystem 
