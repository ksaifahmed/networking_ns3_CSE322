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
WiFi 802.11 Dumbell Network with Two Wired P2P Routers, Wireless Senders, Wired Receivers:
         _                               _
        |   H0------+           *---H5   |
        |			|       			 |
        |	H1------+		    *---H6 	 |
        |			|					 |
Senders	|	H2------R1------R2  *---H7   |	Receivers
(Apps)	|			|		 			 |  (Sinks)
        |	H3------+		    *---H8   |
        |           |                    |
        |_  H4------+           *---H9  _|

        ===============IP Addresses=============
        10.1.3.0 to | --- 10.1.1.0 ----| 10.1.2.0
        10..1.7.0   |                  |

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
#include "ns3/error-rate-model.h"
#include "ns3/yans-error-rate-model.h"

typedef uint32_t uint;

using namespace ns3;


// Some Globals for Throughput Calculation ==================================
FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor;
uint64_t previous_rx = 0; // initial bytes received
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

// OG function
static void
CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
    //*stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void
RxDrop(Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
    // NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
    file->Write(Simulator::Now(), p);
}

//=============sets a single flow from a sender to sink=====================================
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
//=============================================================================================

//========GLOBAL VARS==========

// NO OF NODES ON EACH SIDE OF DUMBBELL
uint32_t nWifi = 6;

uint port = 9000;
uint numPackets = 40000;
uint packetSize = 1024;
std::string transferSpeed = "10Mbps"; // App dataRate

double sinkStart = 0;
double appStart = 1;
double stopTime = 50;
double ErrorRate = 0.0005;
//=============================

int main(int argc, char **argv)
{
    // changing default Congestion Control Algo by adding this line:
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpWestwood"));

    //=============the middle p2p routers=====================
    NodeContainer p2p_routers;
    p2p_routers.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2p_routers);
    //=============================================

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

    //=========================ADDED ERROR RATE ON WIFI SUBNET!!!========================
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (ErrorRate));
    
    //get global node index
    // for(uint32_t i = 0; i < nWifi; i++)
    // {
    //     NS_LOG_UNCOND("\tid = " << wifiStaNodesR.Get(i)->GetId());
    // }

    Config::Set("/NodeList/2/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/PostReceptionErrorModel", PointerValue(em));
    Config::Set("/NodeList/3/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/PostReceptionErrorModel", PointerValue(em));
    Config::Set("/NodeList/4/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/PostReceptionErrorModel", PointerValue(em));    
    Config::Set("/NodeList/5/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/PostReceptionErrorModel", PointerValue(em));
    Config::Set("/NodeList/6/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/PostReceptionErrorModel", PointerValue(em));
    //==================================================================

    //==============Wired nodes on the LEFT net====================
    NodeContainer WiredNodes_Left;
    WiredNodes_Left.Add(p2p_routers.Get(0));
    WiredNodes_Left.Create(nWifi);

    PointToPointHelper left_p2_helper;
    left_p2_helper.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    left_p2_helper.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer wired_net_devices[nWifi];
    for (uint32_t i = 0; i < nWifi; i++)
    {
        wired_net_devices[i] = left_p2_helper.Install(WiredNodes_Left.Get(0), WiredNodes_Left.Get(i + 1));
    }

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
    mobility.Install(wifiStaNodesR);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodeR);
    mobility.Install(WiredNodes_Left);
    //===========================================================

    //======installing protocol stacks on all the nodes==============
    InternetStackHelper stack;
    stack.Install(WiredNodes_Left);
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
    for (uint32_t i = 0; i < nWifi; i++)
    {
        std::string ip_ = "10.1." + std::to_string(i + 3) + ".0";
        ns3::Ipv4Address ip_addr(ip_.c_str());
        address.SetBase(ip_addr, "255.255.255.0");
        address.Assign(wired_net_devices[i]);
    }
    //===============================================

    //================app set up===================

    // tracing one of the sender flows
    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream_cwnd = asciiTraceHelper.CreateFileStream("baseline_westwood.cwnd");
    Ptr<Socket> ns3TcpSocket;

    // init nWifi flows
    for (uint32_t i = 0; i < nWifi; i++)
    {
        ns3TcpSocket = setFlow(InetSocketAddress(right_wireless_interfaces.GetAddress(i), port), port,
                               WiredNodes_Left.Get(i + 1), wifiStaNodesR.Get(i),
                               sinkStart, stopTime,
                               packetSize, numPackets, transferSpeed,
                               appStart, stopTime);
    }
    // set the last (6th) flow, left_first to right_second -- port 9002
    // setFlow(InetSocketAddress(right_wireless_interfaces.GetAddress(2), port + 2), port + 2,
    //         WiredNodes_Left.Get(1), wifiStaNodesR.Get(2),
    //         sinkStart, stopTime,
    //         packetSize, numPackets, transferSpeed,
    //         appStart, stopTime);

    // Ptr<Node> src_node = WiredNodes_Left.Get(5);
    // Ptr<Ipv4> ipv4_ = src_node->GetObject<Ipv4>();
    // Ipv4InterfaceAddress iaddr = ipv4_->GetAddress(1, 0);
    // Ipv4Address ipAddr = iaddr.GetLocal();

    // NS_LOG_UNCOND("SRC: " << ipAddr << ", DEST: " << right_wireless_interfaces.GetAddress(4));
    ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback(&CwndChange, stream_cwnd));
    //===================================================

    //============for routing=======================
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    //==============================================

    //=========Flow mon instantiation=============
    monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(stopTime));
    Simulator::Run();

    //=========Flow Stats calculation==============
    int j = 0;
    float AvgThroughput = 0;
    Time Jitter;
    Time Delay;

    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;
    uint32_t LostPackets = 0;

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin(); iter != stats.end(); ++iter)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first);

        NS_LOG_UNCOND("----Flow ID:" << iter->first);
        NS_LOG_UNCOND("Src Addr" << t.sourceAddress << ", Dst Addr " << t.destinationAddress);
        NS_LOG_UNCOND("Sent Packets =" << iter->second.txPackets);
        NS_LOG_UNCOND("Received Packets =" << iter->second.rxPackets);
        NS_LOG_UNCOND("Lost Packets =" << iter->second.txPackets - iter->second.rxPackets);
        NS_LOG_UNCOND("Packet delivery ratio =" << iter->second.rxPackets * 100.0 / iter->second.txPackets << "%");
        NS_LOG_UNCOND("Packet loss ratio =" << (iter->second.txPackets - iter->second.rxPackets) * 100.0 / iter->second.txPackets << "%");
        NS_LOG_UNCOND("Delay =" << iter->second.delaySum);
        NS_LOG_UNCOND("Jitter =" << iter->second.jitterSum);
        NS_LOG_UNCOND("Throughput =" << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds()) / 1024 << "Kbps");

        SentPackets = SentPackets + (iter->second.txPackets);
        ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
        LostPackets = LostPackets + (iter->second.txPackets - iter->second.rxPackets);
        AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds()) / 1024);
        Delay = Delay + (iter->second.delaySum);
        Jitter = Jitter + (iter->second.jitterSum);

        j = j + 1;
    }

    AvgThroughput = AvgThroughput / j;
    NS_LOG_UNCOND("--------Total Results of the simulation----------" << std::endl);
    NS_LOG_UNCOND("Total sent packets  =" << SentPackets);
    NS_LOG_UNCOND("Total Received Packets =" << ReceivedPackets);
    NS_LOG_UNCOND("Total Lost Packets =" << LostPackets);
    NS_LOG_UNCOND("Packet Loss ratio =" << ((LostPackets * 100) / SentPackets) << "%");
    NS_LOG_UNCOND("Packet delivery ratio =" << ((ReceivedPackets * 100) / SentPackets) << "%");
    NS_LOG_UNCOND("Average Throughput =" << AvgThroughput << "Kbps");
    NS_LOG_UNCOND("End to End Delay =" << Delay);
    NS_LOG_UNCOND("End to End Jitter delay =" << Jitter);
    NS_LOG_UNCOND("Total Flow id " << j);
    monitor->SerializeToXmlFile("manet-routing.xml", true, true);

    Simulator::Destroy();
    return 0;
}