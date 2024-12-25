# !/bin/bash

# CAN test
#sudo ./farviewpcan id4 -l 0 -f 0 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29
#sudo ./farviewpcan g3 -l 0 -f 0 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29
#sudo ./farviewpcan g4r -l 0 -f 0 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29
#sudo ./farviewpcan c3 -l 0 -f 0 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29 -d 55 -r 14 -o 0

# CAN test
#sudo ./farviewpcan $1 -l 0 -f 0 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29
#sudo ./farviewpcan $1 -l 0 -f 0 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29 -d 55 -r 13 -o 0

# Manual test
#sudo ./farviewpcan $1 -l 0 -f 0 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29 -d 55 -r 13 -o 0 -m
sudo ./farviewpcan $1 -m

while [ 0 = 0 ]
do
        sudo ./farview -l 0 -f 1 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29 -p 600
#       sudo ./farview -l 0 -f 0 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29 -p 600
        sudo ./farview -l 1 -f 0
done
