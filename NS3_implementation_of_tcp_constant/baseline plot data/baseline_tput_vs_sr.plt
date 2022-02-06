set terminal png
set output "baseline_tput_vs_sr.png"
set title "Throughput(average) vs S/R"
set xlabel "S/R"
set ylabel "Throughput(Kbps)"

plot "reno_tput_vs_sr.cwnd" using 1:3 with linespoints title "TCP Reno", \
     "westwood_tput_vs_sr.cwnd" using 1:3 with linespoints title "TCP WestWood"