#!/bin/bash
now=$(date +"%y%m%d")
tarfile="TofPetDaq_$now.tar.gz"
tar cvzf $tarfile *.c *.cpp *.h config*.* *.sh Makefile
