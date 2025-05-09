#!/bin/bash

# copy executable to Rpi5
sshpass -p '2024' scp farviewpcan crane@192.168.0.77:/home/crane
#sshpass -p '2024' scp $1 crane@192.168.0.77:/home/crane

# copy source file(s) to Rpi2
# needs to run scp command once before running sshpass
#sshpass -p '2023' scp $1 crane@192.168.0.10:/home/crane/work/autotester/$2
