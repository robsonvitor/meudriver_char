#!/bin/bash

DIR_LOCAL="/home/robson/codigos/"
DIR_REMOTO="root@192.168.56.10:/vagrant/"

rsync -avz -e "ssh -i /home/robson/.ssh/id_rsa.pub -o StrictHostKeyChecking=no" $DIR_LOCAL root@192.168.56.10:/vagrant/
while true; do
  inotifywait -r -q -e modify,attrib,close_write,move,create,delete $DIR_LOCAL
  rsync -avz -e "ssh -i /home/robson/.ssh/id_rsa.pub -o StrictHostKeyChecking=no" $DIR_LOCAL root@192.168.56.10:/vagrant/
  # clear
  date
done
