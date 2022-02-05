set terminal png
set output "baseline_tput_vs_time.png"
set title "Tput vs Time"
set xlabel "Time"
set ylabel "Throughput(Kbps)"

plot "baseline_reno.cwnd" using 1:3 with linespoints title "TCP Reno", \
     "baseline_westwood.cwnd" using 1:3 with linespoints title "TCP WestWood"