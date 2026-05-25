#!/usr/bin/env bash

USERNAME="juandto"

WORKSPACE="/home/juandto/Game-Of-Life"

INPUT="input_X.pbm"

NODES=("148.216.17.81" "148.216.17.85" "148.216.17.87")

echo "Starting deploy..."

for NODE in "${NODES[@]}"; do
  # Creates the directories in the node.
  ssh $USERNAME@$NODE "mkdir -p $WORKSPACE/bin $WORKSPACE/data"
  
  # Sends the executable file.
  scp $WORKSPACE/bin/gol_mpi $NODE:$WORKSPACE/bin/
  
  # Sends the input PBM file.
  scp $WORKSPACE/data/$INPUT $NODE:$WORKSPACE/data/
done

echo "Deploy completed!"