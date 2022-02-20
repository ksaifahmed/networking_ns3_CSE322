/*
===================================================================================
ID: 1705110
TOPOLOGY:
    4 p2p AP Nodes connected in kite formation [AP nodes are vertices of kite]
    Each AP Node houses N Wireless Nodes
    See Report for Detailed Image and Orientation [top, left, right, bottom] of Topology Used!
===================================================================================
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
#include "ns3/yans-wifi-phy.h"


typedef uint32_t uint;

using namespace ns3;


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


//calculates instantaneous tput, cwind
static void
CwndChange_instantaneous (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    //NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
    stats = monitor->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin(); iter != stats.end(); ++iter)
    {
        //Tput of a specific node
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
        if(t.sourceAddress != "10.1.7.2") continue;

        //calculate Kilobits per second
        t_put = (abs(iter->second.rxBytes - previous_rx) * 8.0) / 1000.0;

        //don't print the small values
        if(t_put < 0.01) break;

        //print time, tput and cwnd
        NS_LOG_UNCOND("Time: "<< Simulator::Now().GetSeconds() << ", Tput: " << t_put << ", cwnd: " << oldCwnd);
        previous_rx = iter->second.rxBytes;
        *stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << oldCwnd << "\t" << t_put << std::endl;

        //only one node's Tput
        break;
    }
}

//OG function
static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND("Time: " << Simulator::Now().GetSeconds() << ",\tCwnd: " << newCwnd);
  //*stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void
RxDrop(Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
    //NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
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

//NO OF NODES ON EACH SIDE OF DUMBBELL
uint32_t nWifi = 20;
uint32_t range = 50; //10m
uint32_t flows_by_two = 5;

uint port = 9000;
uint numPackets = 200;
uint packetSize = 1024;
std::string transferSpeed = "10Mbps"; //App dataRate

double sinkStart = 0;
double appStart = 1;
double stopTime = 20;
double ErrorRate = 0.000001;

const char *p2p_data_rate = "15Mbps";
const char *p2p_delay = "0.5ms";
//=============================


int main(int argc, char **argv)
{
    //changing default Congestion Control Algo by adding this line:
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpWestwood"));
    //set max coverage
    Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (range));
    

    //=============the left-top p2p routers=====================
    NodeContainer p2p_LT;
    p2p_LT.Create(2);

    PointToPointHelper pointToPoint1;
    pointToPoint1.SetDeviceAttribute("DataRate", StringValue(p2p_data_rate));
    pointToPoint1.SetChannelAttribute("Delay", StringValue(p2p_delay));

    NetDeviceContainer p2p_lt_Devices;
    p2p_lt_Devices = pointToPoint1.Install(p2p_LT);
    //=============================================    



    //=============the right bottom p2p routers=====================
    NodeContainer p2p_RB;
    p2p_RB.Create(2);

    PointToPointHelper pointToPoint3;
    pointToPoint3.SetDeviceAttribute("DataRate", StringValue(p2p_data_rate));
    pointToPoint3.SetChannelAttribute("Delay", StringValue(p2p_delay));

    NetDeviceContainer p2p_rb_Devices;
    p2p_rb_Devices = pointToPoint3.Install(p2p_RB);
    //=============================================    


    //=============the bottom left p2p routers=====================
    NodeContainer p2p_BL;
    p2p_BL.Add(p2p_RB.Get(1));
    p2p_BL.Add(p2p_LT.Get(0));

    PointToPointHelper pointToPoint4;
    pointToPoint4.SetDeviceAttribute("DataRate", StringValue(p2p_data_rate));
    pointToPoint4.SetChannelAttribute("Delay", StringValue(p2p_delay));

    NetDeviceContainer p2p_bl_Devices;
    p2p_bl_Devices = pointToPoint3.Install(p2p_BL);
    //=============================================    



    //==============Wired nodes on the LEFT net==================== 
    NodeContainer wifiStaNodesL;
    wifiStaNodesL.Create(nWifi/4);
    NodeContainer wifiApNodeL = p2p_LT.Get(0);

    YansWifiChannelHelper left_channel = YansWifiChannelHelper::Default();
    left_channel.AddPropagationLoss("ns3::RangePropagationLossModel");
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


    //==============Wired nodes on the TOP net====================
    NodeContainer wifiStaNodesT;
    wifiStaNodesT.Create(nWifi/4);
    NodeContainer wifiApNodeT = p2p_LT.Get(1);

    YansWifiChannelHelper top_channel = YansWifiChannelHelper::Default();
    top_channel.AddPropagationLoss("ns3::RangePropagationLossModel");
    YansWifiPhyHelper phy_t;
    phy_t.SetChannel(top_channel.Create());

    WifiHelper wifi_t;
    wifi_t.SetRemoteStationManager("ns3::AarfWifiManager");

    WifiMacHelper mac_t;
    Ssid ssid_t = Ssid("ns-3-ssid");
    mac_t.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid_t),
                "ActiveProbing", BooleanValue(false));

    NetDeviceContainer top_staDevices;
    top_staDevices = wifi_t.Install(phy_t, mac_t, wifiStaNodesT);

    mac_t.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid_t));

    NetDeviceContainer top_apDevices;
    top_apDevices = wifi_t.Install(phy_t, mac_t, wifiApNodeT);    
    //==================================================================


    //==============Wireless nodes on the RIGHT net====================
    NodeContainer wifiStaNodesR;
    wifiStaNodesR.Create(nWifi/4);
    NodeContainer wifiApNodeR = p2p_RB.Get(0);

    YansWifiChannelHelper right_channel = YansWifiChannelHelper::Default();
    right_channel.AddPropagationLoss("ns3::RangePropagationLossModel");    
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


    //==============Wired nodes on the BOTTOM net====================
    NodeContainer wifiStaNodesB;
    wifiStaNodesB.Create(nWifi/4);
    NodeContainer wifiApNodeB = p2p_RB.Get(1);

    YansWifiChannelHelper bottom_channel = YansWifiChannelHelper::Default();
    bottom_channel.AddPropagationLoss("ns3::RangePropagationLossModel");
    YansWifiPhyHelper phy_b;
    phy_b.SetChannel(bottom_channel.Create());

    WifiHelper wifi_b;
    wifi_b.SetRemoteStationManager("ns3::AarfWifiManager");

    WifiMacHelper mac_b;
    Ssid ssid_b = Ssid("ns-3-ssid");
    mac_b.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid_b),
                "ActiveProbing", BooleanValue(false));

    NetDeviceContainer bottom_staDevices;
    bottom_staDevices = wifi_b.Install(phy_b, mac_b, wifiStaNodesB);

    mac_b.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid_b));

    NetDeviceContainer bottom_apDevices;
    bottom_apDevices = wifi_b.Install(phy_b, mac_b, wifiApNodeB);    
    //==================================================================


    //========================ADD error model========================
    //get global node index
    // for(uint32_t i = 0; i < nWifi/4; i++)
    // {
    //     NS_LOG_UNCOND("\tid = " << wifiStaNodesL.Get(i)->GetId());
    // }
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (ErrorRate));
    uint32_t index = nWifi/2 + 4;
    while (1)
    {
        std::string path = "/NodeList/" + std::to_string(index) + "/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/PostReceptionErrorModel";
        Config::Set(path.c_str(), PointerValue(em));
        index += nWifi/10;
        if(index > nWifi + 3) break;
    }

    //==================================================================
  


    //==============adding mobility to wireless nodes=============
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(4.0),
                                  "DeltaY", DoubleValue(4.0),
                                  "GridWidth", UintegerValue(5),
                                  "LayoutType", StringValue("RowFirst"));
    
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    
    mobility.Install(wifiStaNodesL);
    mobility.Install(wifiApNodeL);
    
    mobility.Install(wifiStaNodesR);
    mobility.Install(wifiApNodeR);

    mobility.Install(wifiStaNodesT);
    mobility.Install(wifiApNodeT);

    mobility.Install(wifiStaNodesB);   
    mobility.Install(wifiApNodeB);
    //===========================================================  



    //======installing protocol stacks on all the nodes==============
    InternetStackHelper stack;
    stack.Install(wifiApNodeL);
    stack.Install(wifiStaNodesL);
    
    stack.Install(wifiApNodeR);
    stack.Install(wifiStaNodesR);
    
    stack.Install(wifiApNodeT);
    stack.Install(wifiStaNodesT);
    
    stack.Install(wifiApNodeB);
    stack.Install(wifiStaNodesB);        
    //==================================================      


    //================Assign IP Addresses to nets================
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(p2p_lt_Devices);
    address.SetBase("10.1.2.0", "255.255.255.0");

    address.Assign(p2p_rb_Devices);
    address.SetBase("10.1.3.0", "255.255.255.0");
    address.Assign(p2p_bl_Devices); 

    address.SetBase("10.1.4.0", "255.255.255.0");
    address.Assign(left_staDevices);
    address.Assign(left_apDevices);

    address.SetBase("10.1.5.0", "255.255.255.0");
    address.Assign(top_staDevices);
    address.Assign(top_apDevices);

    address.SetBase("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer right_wireless_interfaces;
    right_wireless_interfaces = address.Assign(right_staDevices);
    address.Assign(right_apDevices);    
    
    address.SetBase("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer bottom_wireless_interfaces;
    bottom_wireless_interfaces = address.Assign(bottom_staDevices);
    address.Assign(bottom_apDevices);     
    //===============================================    


    //print node position
    //get global node index
    // for(uint32_t i = 0; i < nWifi/4; i++)
    // {
    //     NS_LOG_UNCOND("id = " << wifiStaNodesL.Get(i)->GetId());
    //     Ptr<Node> object = wifiStaNodesL.Get(i);
    //     Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
    //     NS_ASSERT(position != 0);
    //     Vector pos = position->GetPosition ();
    //     NS_LOG_UNCOND("x="<<pos.x<<", y="<<pos.y<<", z="<<pos.z);
    // }  


    // for(uint32_t i = 0; i < nWifi/4; i++)
    // {
    //     NS_LOG_UNCOND("id = " << wifiStaNodesT.Get(i)->GetId());
    //     Ptr<Node> object = wifiStaNodesT.Get(i);
    //     Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
    //     NS_ASSERT(position != 0);
    //     Vector pos = position->GetPosition ();
    //     NS_LOG_UNCOND("x="<<pos.x<<", y="<<pos.y<<", z="<<pos.z);
    // } 

    // for(uint32_t i = 0; i < nWifi/4; i++)
    // {
    //     NS_LOG_UNCOND("id = " << wifiStaNodesR.Get(i)->GetId());
    //     Ptr<Node> object = wifiStaNodesR.Get(i);
    //     Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
    //     NS_ASSERT(position != 0);
    //     Vector pos = position->GetPosition ();
    //     NS_LOG_UNCOND("x="<<pos.x<<", y="<<pos.y<<", z="<<pos.z);
    // }       
    
    // for(uint32_t i = 0; i < nWifi/4; i++)
    // {
    //     NS_LOG_UNCOND("id = " << wifiStaNodesB.Get(i)->GetId());
    //     Ptr<Node> object = wifiStaNodesB.Get(i);
    //     Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
    //     NS_ASSERT(position != 0);
    //     Vector pos = position->GetPosition ();
    //     NS_LOG_UNCOND("x="<<pos.x<<", y="<<pos.y<<", z="<<pos.z);
    // }

    //==================Setting up the flows: Left to Right, Top to Bottom Nodes=====================
    Ptr<Socket> ns3TcpSocket;    
    for(uint32_t i = 0; i < nWifi/4; i++)
    {
        //Left to Right flow
        ns3TcpSocket = setFlow(InetSocketAddress(right_wireless_interfaces.GetAddress(i), port), port,
                                wifiStaNodesL.Get(i), wifiStaNodesR.Get(i), 
                                sinkStart, stopTime, 
                                packetSize, numPackets, transferSpeed, 
                                appStart, stopTime); 
        //Top to Bottom flow
        setFlow(InetSocketAddress(bottom_wireless_interfaces.GetAddress(i), port), port,
                wifiStaNodesT.Get(i), wifiStaNodesB.Get(i), 
                sinkStart, stopTime, 
                packetSize, numPackets, transferSpeed, 
                appStart, stopTime);                                    
    }

    //last assigned left node
    AsciiTraceHelper asciiTraceHelper;
	Ptr<OutputStreamWrapper> stream_cwnd = asciiTraceHelper.CreateFileStream("baseline_westwood.cwnd");    
    ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback (&CwndChange, stream_cwnd));
    //===================================================


    //============for routing=======================
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    //==============================================


    
    //===========Flow Mon Init===========
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;
    uint32_t LostPackets = 0;

    int j=0;
    double AvgThroughput = 0.0;
    double packet_loss_ratio = 0.0;
    double packet_delivery_ratio = 0.0;
    Time Jitter;
    Time Delay;

    //==============Simulation Start=============
    Simulator::Stop(Seconds(stopTime));
    Simulator::Run();


    //=============Metric Calculation=========
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

        NS_LOG_UNCOND("----Flow ID:" <<iter->first);
        NS_LOG_UNCOND("Src Addr" <<t.sourceAddress << "Dst Addr "<< t.destinationAddress);
        NS_LOG_UNCOND("Sent Packets=" <<iter->second.txPackets);
        NS_LOG_UNCOND("Received Packets =" <<iter->second.rxPackets);
        NS_LOG_UNCOND("Lost Packets =" <<iter->second.txPackets-iter->second.rxPackets);
        packet_delivery_ratio = iter->second.rxPackets*100.0;
        packet_delivery_ratio /= iter->second.txPackets;
        NS_LOG_UNCOND("Packet delivery ratio =" << packet_delivery_ratio << "%");
        packet_loss_ratio = (iter->second.txPackets-iter->second.rxPackets)*100.0;
        packet_loss_ratio /= iter->second.txPackets;
        NS_LOG_UNCOND("Packet loss ratio =" << packet_loss_ratio << "%");
        NS_LOG_UNCOND("Delay =" <<iter->second.delaySum);
        NS_LOG_UNCOND("Jitter =" <<iter->second.jitterSum);
        NS_LOG_UNCOND("Throughput =" <<iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024<<"Kbps");

        SentPackets = SentPackets +(iter->second.txPackets);
        ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
        LostPackets = LostPackets + (iter->second.txPackets-iter->second.rxPackets);
        AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024);
        Delay = Delay + (iter->second.delaySum);
        Jitter = Jitter + (iter->second.jitterSum);

        j = j + 1;
    }
    packet_loss_ratio = ((LostPackets*100.0)/SentPackets);
    packet_delivery_ratio = ((ReceivedPackets*100.0)/SentPackets);
    AvgThroughput = AvgThroughput/j;
    NS_LOG_UNCOND("--------Total Results of the simulation----------"<<std::endl);
    NS_LOG_UNCOND("Total sent packets  =" << SentPackets);
    NS_LOG_UNCOND("Total Received Packets =" << ReceivedPackets);
    NS_LOG_UNCOND("Total Lost Packets =" << LostPackets);
    NS_LOG_UNCOND("Packet Loss ratio =" << packet_loss_ratio << "%");
    NS_LOG_UNCOND("Packet delivery ratio =" << packet_delivery_ratio << "%");
    NS_LOG_UNCOND("Average Throughput =" << AvgThroughput<< "Kbps");
    NS_LOG_UNCOND("End to End Delay =" << Delay);
    NS_LOG_UNCOND("End to End Jitter delay =" << Jitter);
    NS_LOG_UNCOND("Total Flow id " << j);


    Simulator::Destroy();
    return 0;    
    
}