#!/bin/bash
rm *.txt


declare -a nodes=(2, 4, 6, 8, 10)
declare -a flows=(2, 4, 6, 8, 10)
declare -a pps=(1, 2, 3, 4, 5, 10, 20, 30, 40, 100)
declare -a ranges=(2, 3, 4, 5, 10, 20)

for n in ${nodes[@]}; do
    ./waf --run "scratch/task_a_802154_static --n_wirelessNodes=$n --nFlows=2 --nPackets_sec=400 --range=100 --node_var=1"
done

for f in ${flows[@]}; do
    ./waf --run "scratch/task_a_802154_static --n_wirelessNodes=10 --nFlows=$f --nPackets_sec=400 --range=100 --flow_var=1"
done

for p in ${pps[@]}; do
    ./waf --run "scratch/task_a_802154_static --n_wirelessNodes=5 --nFlows=5 --nPackets_sec=$p --range=50 --packet_var=1 --default_ap=1"
done

for r in ${ranges[@]}; do
    ./waf --run "scratch/task_a_802154_static --n_wirelessNodes=5 --nFlows=5 --nPackets_sec=400 --range=$r --range_var=1 --del_y=3"
done