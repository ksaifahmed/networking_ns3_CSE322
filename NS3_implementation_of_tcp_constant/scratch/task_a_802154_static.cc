/*
===================================================================================
ID: 1705110
TOPOLOGY:
    2 wired csma Nodes, one of which is Wireless AP
    The AP Node houses N Wireless Sensor Nodes
    See Report for Detailed Image and Orientation of Topology Used!
===================================================================================
*/

#include <cstdlib>
#include <map>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/mobility-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/propagation-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv6-flow-classifier.h"
#include "ns3/flow-monitor-module.h"
#include <ns3/lr-wpan-error-model.h>
#include <iostream>
using namespace ns3;


//===========================MY APP=================================================
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
    static TypeId tid = TypeId("MyApp")
                            .SetParent<Application>()
                            .SetGroupName("Tutorial")
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
    NS_LOG_UNCOND("Time: " << Simulator::Now().GetSeconds() << ",\tCwnd: " << newCwnd);
    //*stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}
//==================================================================================



//=============sets a single flow from a sender to sink==============================
Ptr<Socket> setFlow(Address sinkAddress, uint sinkPort, Ptr<Node> hostNode, Ptr<Node> sinkNode,
                    double startTime, double stopTime, uint packetSize, uint numPackets, std::string dataRate,
                    double appStartTime, double appStopTime)
{

    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", Inet6SocketAddress(Ipv6Address::GetAny(), sinkPort));
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
//=====================================================================================



//==============GLOBAL VARS=====================
uint16_t n_wirelessNodes = 2;
uint32_t range = 100;
uint32_t nFlows = 2; //keep nFlows <= n_wirelessNodes
uint32_t nPackets_sec = 100;



uint32_t node_var = 0;
uint32_t range_var = 0;
uint32_t flow_var = 0;
uint32_t packet_var = 0;


uint32_t n_wireless_nd_ap;
uint32_t mtu_bytes = 180;
uint32_t tcp_adu_size;
uint64_t data_mbytes = 0;

double sinkStart = 0;
double appStart = 1;
double stopTime = 20;


bool sack = true;
uint16_t sinkPort = 9;

std::string recovery = "ns3::TcpClassicRecovery";
std::string filePrefix;

uint numPackets = 100000; //TOTAL PACKETS
uint packetSize = 1024; //in Bytes
std::string transferSpeed; //App dataRate default

uint32_t del_x = 2;
uint32_t del_y = 2;
uint32_t default_ap = 0;
//===================================================




int main(int argc, char **argv)
{
    CommandLine cmd (__FILE__);
    cmd.AddValue ("nFlows", "no of flows", nFlows);
    cmd.AddValue ("n_wirelessNodes", "no of nodes", n_wirelessNodes);
    cmd.AddValue ("nPackets_sec", "no of packets per sec", nPackets_sec);
    cmd.AddValue ("range", "range of transmission", range);

    cmd.AddValue ("node_var", "enable variable node data to be printed", node_var);
    cmd.AddValue ("range_var", "enable variable range data to be printed", range_var);
    cmd.AddValue ("flow_var", "enable variable flow data to be printed", flow_var);
    cmd.AddValue ("packet_var", "enable variable packet_var data to be printed", packet_var);

    cmd.AddValue ("del_x", "grid_del_x", del_x);
    cmd.AddValue ("del_y", "grid_del_y", del_y);
    cmd.AddValue ("default_ap", "If set to 1: AP-->middle wireless node, else the first one", default_ap);
    
    cmd.Parse (argc, argv);


    //===========================CONFIG SETUP=========================================

    //changing default Congestion Control Algo by adding this line:
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpWestwood"));
    //setting range
    Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (range));



    // FIXING SEGMENT SIZE, GIVES A LOWER PACKET LOSS
    // As Done in tcp-variants-comparison.cc
    Header *header = new Ipv6Header();
    uint32_t ip_header = header->GetSerializedSize();
    header = new TcpHeader();
    uint32_t tcp_header = header->GetSerializedSize();
    tcp_adu_size = mtu_bytes - 20 - (ip_header + tcp_header);
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(tcp_adu_size));
    //===============================================================



    //=========making the Nodes====================================
    n_wireless_nd_ap = n_wirelessNodes + 1; //1 extra AP Node conn. to CSMA
    NodeContainer wsnNodes;
    wsnNodes.Create(n_wireless_nd_ap);

    NodeContainer wiredNodes;
    wiredNodes.Create(1);
    if(default_ap == 1)
        wiredNodes.Add(wsnNodes.Get(n_wirelessNodes/2));
    else wiredNodes.Add(wsnNodes.Get(0));
    //=============================================


  
    //=========setting up mobility=================================
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0),
                                  "MinY", DoubleValue(0),
                                  "DeltaX", DoubleValue(del_x),
                                  "DeltaY", DoubleValue(del_y),
                                  "GridWidth", UintegerValue(4),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wsnNodes);
    //============================================================


    //===============Installing the Netdevices==========================

    // creating a channel with range propagation loss model  
    Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel> ();
    Ptr<RangePropagationLossModel> propModel = CreateObject<RangePropagationLossModel> ();
    Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
    channel->AddPropagationLossModel (propModel);
    channel->SetPropagationDelayModel (delayModel);

    // setting the channel in helper
    LrWpanHelper lrWpanHelper;
    lrWpanHelper.SetChannel(channel);
    NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(wsnNodes);

    lrWpanHelper.AssociateToPan(lrwpanDevices, 0);

    InternetStackHelper internetv6;
    internetv6.Install(wsnNodes);
    internetv6.Install(wiredNodes.Get(0));

    //using sixLowPan because lrWpan does not support TCP
    SixLowPanHelper sixLowPanHelper;
    NetDeviceContainer sixLowPanDevices = sixLowPanHelper.Install(lrwpanDevices);

    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute ("DataRate", StringValue ("1Mbps"));
    csmaHelper.SetChannelAttribute ("Delay", StringValue ("0.5ms"));    
    NetDeviceContainer csmaDevices = csmaHelper.Install(wiredNodes);
    //=======================================================



    //================Assigning Ipv6 Addresses==============================
    Ipv6AddressHelper ipv6;
    ipv6.SetBase(Ipv6Address("2001:cafe::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer wiredDeviceInterfaces;
    wiredDeviceInterfaces = ipv6.Assign(csmaDevices);
    //The AP node in wiredDevices has index = 1
    wiredDeviceInterfaces.SetForwarding(1, true); 
    wiredDeviceInterfaces.SetDefaultRouteInAllNodes(1);

    ipv6.SetBase(Ipv6Address("2001:f00d::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer wsnDeviceInterfaces;
    wsnDeviceInterfaces = ipv6.Assign(sixLowPanDevices);
    // The AP node in wirelessDevices has index = 0    
    wsnDeviceInterfaces.SetForwarding(0, true); 
    wsnDeviceInterfaces.SetDefaultRouteInAllNodes(0);
    //============================================================================



    //================Enabling Mesh Routing========================================
    for (uint32_t i = 0; i < sixLowPanDevices.GetN(); i++)
    {
        Ptr<NetDevice> dev = sixLowPanDevices.Get(i);
        dev->SetAttribute("UseMeshUnder", BooleanValue(true));
        dev->SetAttribute("MeshUnderRadius", UintegerValue(10));
    }
    //=============================================================================



    //===================Setting up FLOWS using MyApp==============================
    Ptr<Socket> ns3TcpSocket;     
        
    int t_speed = (nPackets_sec * packetSize * 8)/1000; //Kilobits per sec
    transferSpeed = std::to_string(t_speed) + "Kbps";
    int j_id = 1;
    //keep nFlows less than n_wirelessNodes
    for (uint32_t i = 1; i <= nFlows; i++)
    {
        //Left to Right flow
        ns3TcpSocket = setFlow(Inet6SocketAddress(wiredDeviceInterfaces.GetAddress(0, 1), sinkPort), sinkPort,
                                wsnNodes.Get(i), wiredNodes.Get(0), 
                                sinkStart, stopTime, 
                                packetSize, numPackets, transferSpeed,
                                appStart, stopTime);

        sinkPort++;
    }
    //=============================================================================



    //================Tracing CWND Change================================
    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("pan.cwnd"); //DUMMY FILESTREAM, USED NEXT LINE
    ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream)); //JUST FOR CONSOLE PRINT/PROGRESS
    //===================================================================




    //============Flow Mon init===============
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;
    uint32_t LostPackets = 0;

    int j=0;
    double AvgThroughput = 0.0;
    double TotalThroughput;
    double packet_loss_ratio = 0.0;
    double packet_delivery_ratio = 0.0;
    Time Jitter;
    Time Delay;


    //============Simulator init====================
    Simulator::Stop(Seconds(stopTime));
    Simulator::Run();


    //=============Metric Calculation=========
    Ptr<Ipv6FlowClassifier> classifier = DynamicCast<Ipv6FlowClassifier> (flowmon.GetClassifier6 ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
        Ipv6FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

        NS_LOG_UNCOND("----Flow ID:" <<iter->first);
        NS_LOG_UNCOND("Src Addr" <<t.sourceAddress << "Dst Addr "<< t.destinationAddress);
        NS_LOG_UNCOND("Sent Packets=" <<iter->second.txPackets);
        NS_LOG_UNCOND("Received Packets =" <<iter->second.rxPackets);
        NS_LOG_UNCOND("Lost Packets =" <<iter->second.txPackets-iter->second.rxPackets);
        packet_delivery_ratio = iter->second.rxPackets*100.0;
        packet_delivery_ratio /= iter->second.txPackets;
        NS_LOG_UNCOND("Packet delivery ratio =" << packet_delivery_ratio << "%");
        packet_loss_ratio = (iter->second.lostPackets)*100.0;
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
    if(SentPackets){
        packet_loss_ratio = ((LostPackets*100.0)/SentPackets);
        packet_delivery_ratio = ((ReceivedPackets*100.0)/SentPackets);
    }

    TotalThroughput = AvgThroughput;
    AvgThroughput = AvgThroughput / j;
    if(ReceivedPackets){
        Delay = Delay / ReceivedPackets;
        Jitter = Jitter / ReceivedPackets;
    }

    
    NS_LOG_UNCOND("--------Total Results of the simulation----------"<<std::endl);
    NS_LOG_UNCOND(transferSpeed << ", Nodes: " << n_wirelessNodes << ", Flows: " << nFlows << ", Packets/sec: " << nPackets_sec << ", Range: " << range);
    
    NS_LOG_UNCOND("Total sent packets  =" << SentPackets);
    NS_LOG_UNCOND("Total Received Packets =" << ReceivedPackets);
    NS_LOG_UNCOND("Total Lost Packets =" << LostPackets);
    NS_LOG_UNCOND("Packet Loss ratio =" << packet_loss_ratio << "%");
    NS_LOG_UNCOND("Packet delivery ratio =" << packet_delivery_ratio << "%");
    NS_LOG_UNCOND("Average Per Node Throughput =" << AvgThroughput << "Kbps");
    NS_LOG_UNCOND("Total Network Throughput =" << TotalThroughput << "Kbps");
    NS_LOG_UNCOND("Average End to End Delay =" << Delay.GetMilliSeconds());
    NS_LOG_UNCOND("Average End to End Jitter delay =" << Jitter.GetMilliSeconds());
    NS_LOG_UNCOND("Total Flow id " << j);
    //=====================================================================


    //OPENED IN APPEND MODE, SO REMOVE FILES FIRST
    if(node_var){
        std::ofstream myfile;
        myfile.open ("taskA_low_rate_node_vs_all.txt", std::ios::app);
        myfile << n_wirelessNodes << " " << AvgThroughput << " ";
        myfile << TotalThroughput << " " << packet_delivery_ratio << " ";
        myfile << packet_loss_ratio << " " << Delay.GetMilliSeconds() << " ";
        myfile << Jitter.GetMilliSeconds() << "\n";
        myfile.close();
    }else if(range_var){
        std::ofstream myfile;
        myfile.open ("taskA_low_rate_range_vs_all.txt", std::ios::app);
        myfile << range << " " << AvgThroughput << " ";
        myfile << TotalThroughput << " " << packet_delivery_ratio << " ";
        myfile << packet_loss_ratio << " " << Delay.GetMilliSeconds() << " ";
        myfile << Jitter.GetMilliSeconds() << "\n";
        myfile.close();
    }else if(flow_var){
        std::ofstream myfile;
        myfile.open ("taskA_low_rate_flow_vs_all.txt", std::ios::app);
        myfile << nFlows << " " << AvgThroughput << " ";
        myfile << TotalThroughput << " " << packet_delivery_ratio << " ";
        myfile << packet_loss_ratio << " " << Delay.GetMilliSeconds() << " ";
        myfile << Jitter.GetMilliSeconds() << "\n";
        myfile.close();
    }else if(packet_var){
        std::ofstream myfile;
        myfile.open ("taskA_low_rate_packet_vs_all.txt", std::ios::app);
        myfile << nPackets_sec << " " << AvgThroughput << " ";
        myfile << TotalThroughput << " " << packet_delivery_ratio << " ";
        myfile << packet_loss_ratio << " " << Delay.GetMilliSeconds() << " ";
        myfile << Jitter.GetMilliSeconds() << "\n";
        myfile.close();
    }

    
    
    Simulator::Destroy();
    return 0;
}
