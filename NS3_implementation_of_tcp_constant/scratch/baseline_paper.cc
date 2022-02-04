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
        |   H0--*           +------H5    |
        |			        |			 |		
		|	H1--*   		+------H6	 |
		|					|			 |
Senders	|	H2--*   R1------R2-----H7	 |	Receivers
(Apps)	|					|			 |  (Sinks)
		|	H3--*   		+------H8  	 |
        |                   |            |
        |_  H4--*           +------H9   _|

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
    uint32_t nWifi = 5;
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


    //==============Wired nodes on the LEFT net====================
    NodeContainer WiredNodes_Left;
    WiredNodes_Left.Add(p2p_routers.Get(0));
    WiredNodes_Left.Create(nWifi);

    PointToPointHelper left_p2_helper;
    left_p2_helper.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    left_p2_helper.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer wired_net_devices[nWifi];
    for(int i = 0; i < nWifi; i++) {
        wired_net_devices[i] = left_p2_helper.Install(WiredNodes_Left.Get(0), WiredNodes_Left.Get(i+1));
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
    for(int i = 0; i < nWifi; i++) {
        std::string ip_ = "10.1." + std::to_string(i+3) + ".0";
        ns3::Ipv4Address ip_addr(ip_.c_str());
        address.SetBase(ip_addr, "255.255.255.0");
        address.Assign(wired_net_devices[i]);
    }
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
        echoClient.Install(WiredNodes_Left.Get(nWifi - 2));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    //===================================================


    //============for routing=======================
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    //==============================================


    Simulator::Stop(Seconds(10.0));

    if (tracing)
    {
        phy_r.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        pointToPoint.EnablePcapAll("third");
        phy_r.EnablePcap("third", right_apDevices.Get(0));        

    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}