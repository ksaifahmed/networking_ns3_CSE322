set terminal png
set output "LRS_Nodes_vs_Tput.png"
set title "Varying Nodes in Low Rate 802.11"
set xlabel "Throughput(Kbps)"
set ylabel "Nodes"
plot "taskA_low_rate_node_vs_all.txt" using 3:1 with linespoints title "Nodes vs Throughput"

set terminal png
set output "LRS_Nodes_vs_PDR.png"
set title "Varying Nodes in Low Rate 802.11"
set xlabel "PacketDeliveryRatio (%)"
set ylabel "Nodes"
plot "taskA_low_rate_node_vs_all.txt" using 4:1 with linespoints title "Nodes vs PacketDeliveryRatio"

set terminal png
set output "LRS_Nodes_vs_PLR.png"
set title "Varying Nodes in Low Rate 802.11"
set xlabel "PacketLossRatio (%)"
set ylabel "Nodes"
plot "taskA_low_rate_node_vs_all.txt" using 5:1 with linespoints title "Nodes vs PacketLossRatio"

set terminal png
set output "LRS_Nodes_vs_D.png"
set title "Varying Nodes in Low Rate 802.11"
set xlabel "AverageDelay (ms)"
set ylabel "Nodes"
plot "taskA_low_rate_node_vs_all.txt" using 6:1 with linespoints title "Nodes vs AverageDelay"

set terminal png
set output "LRS_Nodes_vs_J.png"
set title "Varying Nodes in Low Rate 802.11"
set xlabel "Jitter (ms)"
set ylabel "Nodes"
plot "taskA_low_rate_node_vs_all.txt" using 7:1 with linespoints title "Nodes vs Jitter"







set terminal png
set output "LRS_Flow_vs_Tput.png"
set title "Varying Flow in Low Rate 802.11"
set xlabel "Throughput(Kbps)"
set ylabel "Flow"
plot "taskA_low_rate_flow_vs_all.txt" using 3:1 with linespoints title "Flow vs Throughput"

set terminal png
set output "LRS_Flow_vs_PDR.png"
set title "Varying Flow in Low Rate 802.11"
set xlabel "PacketDeliveryRatio (%)"
set ylabel "Flow"
plot "taskA_low_rate_flow_vs_all.txt" using 4:1 with linespoints title "Flow vs PacketDeliveryRatio"

set terminal png
set output "LRS_Flow_vs_PLR.png"
set title "Varying Flow in Low Rate 802.11"
set xlabel "PacketLossRatio (%)"
set ylabel "Flow"
plot "taskA_low_rate_flow_vs_all.txt" using 5:1 with linespoints title "Flow vs PacketLossRatio"

set terminal png
set output "LRS_Flow_vs_D.png"
set title "Varying Flow in Low Rate 802.11"
set xlabel "AverageDelay (ms)"
set ylabel "Flow"
plot "taskA_low_rate_flow_vs_all.txt" using 6:1 with linespoints title "Flow vs AverageDelay"

set terminal png
set output "LRS_Flow_vs_J.png"
set title "Varying Flow in Low Rate 802.11"
set xlabel "Jitter (ms)"
set ylabel "Flow"
plot "taskA_low_rate_flow_vs_all.txt" using 7:1 with linespoints title "Flow vs Jitter"







set terminal png
set output "LRS_PacketsPerSec_vs_Tput.png"
set title "Varying PacketsPerSec in Low Rate 802.11"
set xlabel "Throughput(Kbps)"
set ylabel "PacketsPerSec"
plot "taskA_low_rate_packet_vs_all.txt" using 3:1 with linespoints title "PacketsPerSec vs Throughput"

set terminal png
set output "LRS_PacketsPerSec_vs_PDR.png"
set title "Varying PacketsPerSec in Low Rate 802.11"
set xlabel "PacketDeliveryRatio (%)"
set ylabel "PacketsPerSec"
plot "taskA_low_rate_packet_vs_all.txt" using 4:1 with linespoints title "PacketsPerSec vs PacketDeliveryRatio"

set terminal png
set output "LRS_PacketsPerSec_vs_PLR.png"
set title "Varying PacketsPerSec in Low Rate 802.11"
set xlabel "PacketLossRatio (%)"
set ylabel "PacketsPerSec"
plot "taskA_low_rate_packet_vs_all.txt" using 5:1 with linespoints title "PacketsPerSec vs PacketLossRatio"

set terminal png
set output "LRS_PacketsPerSec_vs_D.png"
set title "Varying PacketsPerSec in Low Rate 802.11"
set xlabel "AverageDelay (ms)"
set ylabel "PacketsPerSec"
plot "taskA_low_rate_packet_vs_all.txt" using 6:1 with linespoints title "PacketsPerSec vs AverageDelay"

set terminal png
set output "LRS_PacketsPerSec_vs_J.png"
set title "Varying PacketsPerSec in Low Rate 802.11"
set xlabel "Jitter (ms)"
set ylabel "PacketsPerSec"
plot "taskA_low_rate_packet_vs_all.txt" using 7:1 with linespoints title "PacketsPerSec vs Jitter"








set terminal png
set output "LRS_Range_vs_Tput.png"
set title "Varying Range in Low Rate 802.11"
set xlabel "Throughput(Kbps)"
set ylabel "Range"
plot "taskA_low_rate_range_vs_all.txt" using 3:1 with linespoints title "Range vs Throughput"

set terminal png
set output "LRS_Range_vs_PDR.png"
set title "Varying Range in Low Rate 802.11"
set xlabel "PacketDeliveryRatio (%)"
set ylabel "Range"
plot "taskA_low_rate_range_vs_all.txt" using 4:1 with linespoints title "Range(m) vs PacketDeliveryRatio"

set terminal png
set output "LRS_Range_vs_PLR.png"
set title "Varying Range in Low Rate 802.11"
set xlabel "PacketLossRatio (%)"
set ylabel "Range"
plot "taskA_low_rate_range_vs_all.txt" using 5:1 with linespoints title "Range(m) vs PacketLossRatio"

set terminal png
set output "LRS_Range_vs_D.png"
set title "Varying Range in Low Rate 802.11"
set xlabel "AverageDelay (ms)"
set ylabel "Range"
plot "taskA_low_rate_range_vs_all.txt" using 6:1 with linespoints title "Range(m) vs AverageDelay"

set terminal png
set output "LRS_Range_vs_J.png"
set title "Varying Range in Low Rate 802.11"
set xlabel "Jitter (ms)"
set ylabel "Range"
plot "taskA_low_rate_range_vs_all.txt" using 7:1 with linespoints title "Range(m) vs Jitter"