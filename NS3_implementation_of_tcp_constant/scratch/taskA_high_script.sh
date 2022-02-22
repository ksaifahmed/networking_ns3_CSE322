#!/bin/bash
rm *.txt

# NOTE: THIS SHELL SCRIPT RUNS 20 SIMULATIONS, SO HAVE PATIENCE!!
# 100 Nodes take a long time!

declare -a nodes=(20, 40, 60, 80, 100)
declare -a flows=(10, 20, 30, 40, 50)
declare -a pps=(100, 200, 300, 400, 500)
declare -a ranges=(10, 20, 30, 40, 50)

for n in ${nodes[@]}; do
    ./waf --run "scratch/task_a_80211_static --nWifi=$n --nFlows=10 --nPackets_sec=400 --range=50 --node_var=1"
done

for f in ${flows[@]}; do
    ./waf --run "scratch/task_a_80211_static --nWifi=100 --nFlows=$f --nPackets_sec=400 --range=50 --flow_var=1"
done

for p in ${pps[@]}; do
    ./waf --run "scratch/task_a_80211_static --nWifi=100 --nFlows=50 --nPackets_sec=$p --range=50 --packet_var=1"
done

for r in ${ranges[@]}; do
    ./waf --run "scratch/task_a_80211_static --nWifi=60 --nFlows=30 --nPackets_sec=400 --range=$r --range_var=1"
done