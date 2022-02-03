/*
 * Copyright (C) 2013 - Dhruv Kohli <codechiggum at gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * 
 * This code heavily borrows from ns3 itself which are copyright of their
 * respective authors and redistributable under the same conditions.
 *
 */

/*
Application Detail:
Analyse and compare TCP Reno, TCP Vegas, and TCP Fack (i.e. Reno TCP with "forward
acknowledgment") performance. Select a Dumbbell topology with two routers R1 and R2 connected by a
(10 Mbps, 50 ms) wired link. Each of the routers is connected to 3 hosts i.e., H1 to H3 (i.e. senders) are
connected to R1 and H4 to H6 (i.e. receivers) are connected to R2. The hosts are attached with (100 Mbps,
20ms) links. Both the routers use drop-tail queues with queue size set according to bandwidth-delay product.
Senders (i.e. H1, H2 and H3) are attached with TCP Reno, TCP Vegas, and TCP Fack agents respectively.
Choose a packet size of 1.2KB and perform the following task. Make appropriate assumptions wherever
necessary.
a. Start only one flow and analyse the throughput over sufficiently long duration. Mention how do you
select the duration. Plot of evolution of congestion window over time. Perform this experiment
with flows attached to all the three sending agents.
b. Next, start 2 other flows sharing the bottleneck while the first one is in progress and measure the
throughput (in Kbps) of each flow. Plot the throughput and congestion window of each flow at
steady-state. What is the maximum throughput of each flow? 
c. Measure the congestion loss and goodput over the duration of the experiment for each flow. 
Implementation detail:
		 _								_
		|	H1------+		+------H4	 |
		|			|		|			 |
Senders	|	H2------R1------R2-----H5	 |	Receivers
		|			|		|			 |
		|_	H3------+		+------H6	_|
	Representation in code:
	H1(n0), H2(n1), H3(n2), H4(n3), H5(n4), H6(n5), R1(n6), R2(n7) :: n stands for node
	Dumbbell topology is used with 
	H1, H2, H3 on left side of dumbbell,
	H4, H5, H6 on right side of dumbbell,
	and routers R1 and R2 form the bridge of dumbbell.
	H1 is attached with TCP Reno agent.
	H2 is attached with TCP Vegas agent.
	H3 is attached with TCP Fack agent.
	Links:
	H1R1/H2R1/H3R1/H4R2/H5R2/H6R2: P2P with 100Mbps and 20ms.
	R1R2: (dumbbell bridge) P2P with 10Mbps and 50ms.
	packet size: 1.2KB.
	Number of packets decided by Bandwidth delay product:
	i.e. #packets = Bandwidth*Delay(in bits)
	Therefore, max #packets (HiRj) = 100Mbps*20ms = 2000000
	and max #packets (R1R2) = 10Mbps*50ms = 500000
*/
#include <string>
#include <fstream>
#include <cstdlib>
#include <map>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

typedef uint32_t uint;

using namespace ns3;

#define ERROR 0.000001

//Some Globals for Throughput Calculation ==================================
FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor;
uint64_t previous_rx = 0; //initial bytes received
std::map<FlowId, FlowMonitor::FlowStats> stats;
bool first = false, first_pos = false;
double t_put = 0.0, temp = 0.0;
//===========================================================================

class App : public Application
{
public:
    App();
    virtual ~App();

    /**
   * Register this type.
   * \return The TypeId.
   */
    static TypeId GetTypeId(void);
    void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void ScheduleTx(void);
    void SendPacket(void);

    Ptr<Socket> m_socket;
    Address m_peer;
    uint32_t m_packetSize;
    uint32_t m_nPackets;
    DataRate m_dataRate;
    EventId m_sendEvent;
    bool m_running;
    uint32_t m_packetsSent;
};

App::App()
    : m_socket(0),
      m_peer(),
      m_packetSize(0),
      m_nPackets(0),
      m_dataRate(0),
      m_sendEvent(),
      m_running(false),
      m_packetsSent(0)
{
}

App::~App()
{
    m_socket = 0;
}

/* static */
TypeId App::GetTypeId(void)
{
    static TypeId tid = TypeId("App")
                            .SetParent<Application>()
                            .SetGroupName("KSA")
                            .AddConstructor<App>();
    return tid;
}

void App::Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_nPackets = nPackets;
    m_dataRate = dataRate;
}

void App::StartApplication(void)
{
    m_running = true;
    m_packetsSent = 0;
    if (InetSocketAddress::IsMatchingType(m_peer))
    {
        m_socket->Bind();
    }
    else
    {
        m_socket->Bind6();
    }
    m_socket->Connect(m_peer);
    SendPacket();
}

void App::StopApplication(void)
{
    m_running = false;

    if (m_sendEvent.IsRunning())
    {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket)
    {
        m_socket->Close();
    }
}

void App::SendPacket(void)
{
    Ptr<Packet> packet = Create<Packet>(m_packetSize);
    m_socket->Send(packet);

    if (++m_packetsSent < m_nPackets)
    {
        ScheduleTx();
    }
}

void App::ScheduleTx(void)
{
    if (m_running)
    {
        Time tNext(Seconds(m_packetSize * 8 / static_cast<double>(m_dataRate.GetBitRate())));
        m_sendEvent = Simulator::Schedule(tNext, &App::SendPacket, this);
    }
}

static void
CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    //NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
    stats = monitor->GetFlowStats();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin(); iter != stats.end(); ++iter)
    {
        t_put = (abs(iter->second.rxBytes - previous_rx) * 8.0) / 1000.0;
        NS_LOG_UNCOND("Throughput =" << t_put << " Kbps, time: " << Simulator::Now().GetSeconds());
        previous_rx = iter->second.rxBytes;
        break;
    }
    *stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << oldCwnd << "\t" << t_put << std::endl;
}

static void
RxDrop(Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
    //NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
    file->Write(Simulator::Now(), p);
}

Ptr<Socket> setFlow(Address sinkAddress, uint sinkPort, Ptr<Node> hostNode, Ptr<Node> sinkNode,
                    double startTime, double stopTime, uint packetSize, uint numPackets, std::string dataRate,
                    double appStartTime, double appStopTime)
{

    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    ApplicationContainer sinkApps = packetSinkHelper.Install(sinkNode);
    sinkApps.Start(Seconds(startTime));
    sinkApps.Stop(Seconds(stopTime));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(hostNode, TcpSocketFactory::GetTypeId());

    Ptr<App> app = CreateObject<App>();
    app->Setup(ns3TcpSocket, sinkAddress, packetSize, numPackets, DataRate(dataRate));
    hostNode->AddApplication(app);
    app->SetStartTime(Seconds(appStartTime));
    app->SetStopTime(Seconds(appStopTime));

    return ns3TcpSocket;
}

int main(int argc, char **argv)
{
    //changing default Congestion Control Algo by adding this line:
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpLinuxReno"));

    bool verbose = true;
    uint32_t nWifi = 3;
    bool tracing = false;

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    //=============the middle p2p routers=====================
    NodeContainer p2p_routers;
    p2p_routers.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2p_routers);
    //=============================================

    //==============Wireless nodes on the LEFT net====================
    NodeContainer wifiStaNodesL;
    wifiStaNodesL.Create(nWifi);
    NodeContainer wifiApNodeL = p2p_routers.Get(0);

    YansWifiChannelHelper left_channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy_l;
    phy_l.SetChannel(left_channel.Create());

    WifiHelper wifi_l;
    wifi_l.SetRemoteStationManager("ns3::AarfWifiManager");

    WifiMacHelper mac_l;
    Ssid ssid_l = Ssid("ns-3-ssid");
    mac_l.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid_l),
                "ActiveProbing", BooleanValue(false));

    NetDeviceContainer left_staDevices;
    left_staDevices = wifi_l.Install(phy_l, mac_l, wifiStaNodesL);

    mac_l.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid_l));

    NetDeviceContainer left_apDevices;
    left_apDevices = wifi_l.Install(phy_l, mac_l, wifiApNodeL);
    //==================================================================


    //==============Wireless nodes on the RIGHT net====================
    NodeContainer wifiStaNodesR;
    wifiStaNodesR.Create(nWifi);
    NodeContainer wifiApNodeR = p2p_routers.Get(1);

    YansWifiChannelHelper right_channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy_r;
    phy_r.SetChannel(right_channel.Create());

    WifiHelper wifi_r;
    wifi_r.SetRemoteStationManager("ns3::AarfWifiManager");

    WifiMacHelper mac_r;
    Ssid ssid_r = Ssid("ns-3-ssid");
    mac_r.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid_r),
                "ActiveProbing", BooleanValue(false));

    NetDeviceContainer right_staDevices;
    right_staDevices = wifi_r.Install(phy_r, mac_r, wifiStaNodesR);

    mac_r.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid_r));

    NetDeviceContainer right_apDevices;
    right_apDevices = wifi_r.Install(phy_r, mac_r, wifiApNodeR);
    //==================================================================


    //==============adding mobility to wireless nodes=============
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(5.0),
                                  "DeltaY", DoubleValue(10.0),
                                  "GridWidth", UintegerValue(3),
                                  "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(wifiStaNodesL);
    mobility.Install(wifiStaNodesR);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodeL);
    mobility.Install(wifiApNodeR);
    //===========================================================



    //======installing protocol stacks on all the nodes==============
    InternetStackHelper stack;
    stack.Install(wifiApNodeL);
    stack.Install(wifiStaNodesL);
    stack.Install(wifiApNodeR);
    stack.Install(wifiStaNodesR);
    //==================================================


    //================Assign IP Addresses to nets================
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer right_wireless_interfaces;
    right_wireless_interfaces = address.Assign(right_staDevices);
    address.Assign(right_apDevices);

    address.SetBase("10.1.3.0", "255.255.255.0");
    address.Assign(left_staDevices);
    address.Assign(left_apDevices);
    //===============================================


    //================app set up===================
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(wifiStaNodesR.Get(nWifi-1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(right_wireless_interfaces.GetAddress(nWifi-1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps =
        echoClient.Install(wifiStaNodesL.Get(nWifi - 1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    //===================================================


    //============for routing=======================
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    //==============================================


    Simulator::Stop(Seconds(10.0));

    if (tracing)
    {
        phy_l.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        pointToPoint.EnablePcapAll("third");
        phy_l.EnablePcap("third", left_apDevices.Get(0));

        phy_r.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        phy_r.EnablePcap("third", right_apDevices.Get(0));        

    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}