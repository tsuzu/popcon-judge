#! /bin/bash

g++ -std=c++14 -O2 -o ./bin/popj ./src/main.cpp /usr/local/lib/libsioclient.a -I /usr/local/include -L /usr/local/lib -lcpprest -lboost_system -lboost_program_options -lboost_filesystem 
