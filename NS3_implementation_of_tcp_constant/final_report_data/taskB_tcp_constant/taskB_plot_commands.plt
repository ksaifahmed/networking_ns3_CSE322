set terminal png
set output "Throughput_v_Time.png"
set title "Throughput vs Time"
set xlabel "Time(s)"
set ylabel "Throughput(Kbps)"

plot "reno_vs_time.txt" using 1:3 with linespoints pointinterval 20 title "TCP Reno", \
     "westwood_vs_time.txt" using 1:3 with linespoints pointinterval 20 title "TCP WestWood", \
     "constant_vs_time.txt" using 1:3 with linespoints pointinterval 20 title "TCP Constant"





set terminal png
set output "CWND_vs_Time.png"
set title "Cwnd vs Time (zoomed in on slow start)"
set xlabel "Time(s)"
set ylabel "Cwnd(Bytes)"
set xrange [0:12]

plot "reno_vs_time.txt" using 1:2 with linespoints title "TCP Reno", \
     "westwood_vs_time.txt" using 1:2 with linespoints title "TCP WestWood", \
     "constant_vs_time.txt" using 1:2 with linespoints title "TCP Constant"



set terminal png
set output "Throughput_v_SR.png"
set title "Throughput vs S/R Pair"
set xlabel "S/R Pair"
set ylabel "Throughput(Kbps)"
set xrange [5:8]

plot "reno_sr_vs_tput.txt" using 1:2 with linespoints title "TCP Reno", \
     "westwood_sr_vs_tput.txt" using 1:2 with linespoints title "TCP WestWood", \
     "constant_sr_vs_tput.txt" using 1:2 with linespoints title "TCP Constant"