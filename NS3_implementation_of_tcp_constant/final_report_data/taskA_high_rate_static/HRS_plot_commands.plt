set terminal png
set output "HRS_Tput_vs_Nodes.png"
set title "Varying Nodes in High Rate 802.11"
set ylabel "Throughput(Kbps)"
set xlabel "Nodes"
plot "taskA_high_rate_node_vs_all.txt" using 1:3 with linespoints title "Throughput vs Nodes"

set terminal png
set output "HRS_PDR_vs_Nodes.png"
set title "Varying Nodes in High Rate 802.11"
set ylabel "PacketDeliveryRatio (%)"
set xlabel "Nodes"
plot "taskA_high_rate_node_vs_all.txt" using 1:4 with linespoints title "PacketDeliveryRatio vs Nodes"

set terminal png
set output "HRS_PLR_vs_Nodes.png"
set title "Varying Nodes in High Rate 802.11"
set ylabel "PacketLossRatio (%)"
set xlabel "Nodes"
plot "taskA_high_rate_node_vs_all.txt" using 1:5 with linespoints title "PacketLossRatio vs Nodes"

set terminal png
set output "HRS_D_vs_Nodes.png"
set title "Varying Nodes in High Rate 802.11"
set ylabel "AverageDelay (ms)"
set xlabel "Nodes"
plot "taskA_high_rate_node_vs_all.txt" using 1:6 with linespoints title "AverageDelay vs Nodes"

set terminal png
set output "HRS_J_vs_Nodes.png"
set title "Varying Nodes in High Rate 802.11"
set ylabel "Jitter (ms)"
set xlabel "Nodes"
plot "taskA_high_rate_node_vs_all.txt" using 1:7 with linespoints title "Jitter vs Nodes"







set terminal png
set output "HRS_Tput_vs_Flow.png"
set title "Varying Flow in High Rate 802.11"
set ylabel "Throughput(Kbps)"
set xlabel "Flow"
plot "taskA_high_rate_flow_vs_all.txt" using 1:3 with linespoints title "Throughput vs Flow"

set terminal png
set output "HRS_PDR_vs_Flow.png"
set title "Varying Flow in High Rate 802.11"
set ylabel "PacketDeliveryRatio (%)"
set xlabel "Flow"
plot "taskA_high_rate_flow_vs_all.txt" using 1:4 with linespoints title "PacketDeliveryRatio vs Flow"

set terminal png
set output "HRS_PLR_vs_Flow.png"
set title "Varying Flow in High Rate 802.11"
set ylabel "PacketLossRatio (%)"
set xlabel "Flow"
plot "taskA_high_rate_flow_vs_all.txt" using 1:5 with linespoints title "PacketLossRatio vs Flow"

set terminal png
set output "HRS_D_vs_Flow.png"
set title "Varying Flow in High Rate 802.11"
set ylabel "AverageDelay (ms)"
set xlabel "Flow"
plot "taskA_high_rate_flow_vs_all.txt" using 1:6 with linespoints title "AverageDelay vs Flow"

set terminal png
set output "HRS_J_vs_Flow.png"
set title "Varying Flow in High Rate 802.11"
set ylabel "Jitter (ms)"
set xlabel "Flow"
plot "taskA_high_rate_flow_vs_all.txt" using 1:7 with linespoints title "Jitter vs Flow"







set terminal png
set output "HRS_Tput_vs_Range.png"
set title "Varying Range in High Rate 802.11"
set ylabel "Throughput(Kbps)"
set xlabel "Range"
plot "taskA_high_rate_range_vs_all.txt" using 1:3 with linespoints title "Throughput vs Range(m)"

set terminal png
set output "HRS_PDR_vs_Range.png"
set title "Varying Range in High Rate 802.11"
set ylabel "PacketDeliveryRatio (%)"
set xlabel "Range"
plot "taskA_high_rate_range_vs_all.txt" using 1:4 with linespoints title "PacketDeliveryRatio vs Range(m)"

set terminal png
set output "HRS_PLR_vs_Range.png"
set title "Varying Range in High Rate 802.11"
set ylabel "PacketLossRatio (%)"
set xlabel "Range"
plot "taskA_high_rate_range_vs_all.txt" using 1:5 with linespoints title "PacketLossRatio vs Range(m)"

set terminal png
set output "HRS_D_vs_Range.png"
set title "Varying Range in High Rate 802.11"
set ylabel "AverageDelay (ms)"
set xlabel "Range"
plot "taskA_high_rate_range_vs_all.txt" using 1:6 with linespoints title "AverageDelay vs Range(m)"

set terminal png
set output "HRS_J_vs_Range.png"
set title "Varying Range in High Rate 802.11"
set ylabel "Jitter (ms)"
set xlabel "Range"
plot "taskA_high_rate_range_vs_all.txt" using 1:7 with linespoints title "Jitter vs Range(m)"






set terminal png
set output "HRS_Tput_vs_PacketsPerSec.png"
set title "Varying PacketsPerSec in High Rate 802.11"
set ylabel "Throughput(Kbps)"
set xlabel "Range"
plot "taskA_high_rate_packet_vs_all.txt" using 1:3 with linespoints title "Throughput vs PacketsPerSec"

set terminal png
set output "HRS_PDR_vs_PacketsPerSec.png"
set title "Varying PacketsPerSec in High Rate 802.11"
set ylabel "PacketDeliveryRatio (%)"
set xlabel "PacketsPerSec"
plot "taskA_high_rate_packet_vs_all.txt" using 1:4 with linespoints title "PacketDeliveryRatio vs PacketsPerSec"

set terminal png
set output "HRS_PLR_vs_PacketsPerSec.png"
set title "Varying PacketsPerSec in High Rate 802.11"
set ylabel "PacketLossRatio (%)"
set xlabel "PacketsPerSec"
plot "taskA_high_rate_packet_vs_all.txt" using 1:5 with linespoints title "PacketLossRatio vs PacketsPerSec"

set terminal png
set output "HRS_D_vs_PacketsPerSec.png"
set title "Varying PacketsPerSec in High Rate 802.11"
set ylabel "AverageDelay (ms)"
set xlabel "PacketsPerSec"
plot "taskA_high_rate_packet_vs_all.txt" using 1:6 with linespoints title "AverageDelay vs PacketsPerSec"

set terminal png
set output "HRS_J_vs_PacketsPerSec.png"
set title "Varying PacketsPerSec in High Rate 802.11"
set ylabel "Jitter (ms)"
set xlabel "PacketsPerSec"
plot "taskA_high_rate_packet_vs_all.txt" using 1:7 with linespoints title "Jitter vs PacketsPerSec"