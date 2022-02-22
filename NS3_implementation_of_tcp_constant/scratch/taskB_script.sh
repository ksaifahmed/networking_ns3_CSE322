#!/bin/bash
rm *.txt

# HAVE PATIENCE, THE SIMULATIONS HERE TAKE A LONG TIME!

# THROUGHPUT vs TIME && CWND vs TIME
./waf --run "scratch/taskB --algo=\"ns3::TcpLinuxReno\" --cwndFile=\"reno_vs_time.txt\""
./waf --run "scratch/taskB --algo=\"ns3::TcpWestwood\" --cwndFile=\"westwood_vs_time.txt\""
./waf --run "scratch/taskB --algo=\"ns3::TcpConstant\" --cwndFile=\"constant_vs_time.txt\""


# SR VS THROUGHPUT
declare -a sr_arr=(5, 6, 7, 8)

for sr in ${sr_arr[@]}; do
    ./waf --run "scratch/taskB --algo=\"ns3::TcpLinuxReno\" --srFile=\"reno_sr_vs_tput.txt\" --srPair=$sr --mode=1"
done


for sr in ${sr_arr[@]}; do
    ./waf --run "scratch/taskB --algo=\"ns3::TcpWestwood\" --srFile=\"westwood_sr_vs_tput.txt\" --srPair=$sr --mode=1"
done


for sr in ${sr_arr[@]}; do
    ./waf --run "scratch/taskB --algo=\"ns3::TcpConstant\" --srFile=\"constant_sr_vs_tput.txt\" --srPair=$sr --mode=1"
done

file="dummy.cwnd"
if [ -f "$file" ] ; then
    rm "$file"
fi

gnuplot "taskB_plot_commands.plt"