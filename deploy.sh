#!/bin/bash

# Template

USERNAME=""

WORKSPACE="/home/"

NODES=("192.168.1.101" "192.168.1.102" "192.168.1.108" "192.168.1.109" "192.168.1.110")

echo "Starting deploy..."

for NODE in "${NODES[@]}"; do
  # Creates the directories in the node.
  ssh $USERNAME@$NODE "mkdir -p $WORKSPACE/bin $WORKSPACE/data"
  
  # Sends the executable file.
  scp $WORKSPACE/bin/gol_mpi $NODE:$WORKSPACE/bin/
  
  # Sends the input PBM file.
  scp $WORKSPACE/data/input.pbm $NODE:$WORKSPACE/data/
done

echo "Deploy completed!"