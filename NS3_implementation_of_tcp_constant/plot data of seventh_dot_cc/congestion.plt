set terminal png
set output "Throughput_v_Cwnd.png"
set title "Throughput vs Cwnd"
set xlabel "Congestion Window"
set ylabel "Throughput(Kbps)"

plot "reno.cwnd" using 2:3 with linespoints title "TCP Reno", \
     "westwood.cwnd" using 2:3 with linespoints title "TCP WestWood"