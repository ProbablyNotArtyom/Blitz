#!/bin/bash

export LIBCUPL=$PWD/cupl/Atmel.dl
wine cupl/cupl.exe $1 -j

