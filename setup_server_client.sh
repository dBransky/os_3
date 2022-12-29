#!/bin/bash
pwd=$(pwd)
echo "alias server ='$pwd ./server 8003 5 5 dt'" >> ~/.bashrc
echo "alias client ='$pwd ./clinet localhost 8003 home.html'" >> ~/.bashrc
source ~/.bashrc