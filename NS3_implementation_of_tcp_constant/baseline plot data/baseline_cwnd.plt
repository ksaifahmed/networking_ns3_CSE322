set terminal png
set output "baseline_cwnd_vs_time.png"
set title "Cwnd vs Time"
set xlabel "Time"
set ylabel "Congestion Window"

plot "baseline_reno.cwnd" using 1:2 with linespoints title "TCP Reno", \
     "baseline_westwood.cwnd" using 1:2 with linespoints title "TCP WestWood"