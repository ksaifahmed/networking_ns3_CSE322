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
#include "ns3/flow-monitor-helper.h"
#include <ns3/lr-wpan-error-model.h>
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
uint32_t n_wireless_nd_ap;
uint16_t sinkPort = 9;

uint32_t mtu_bytes = 180;
uint32_t tcp_adu_size;
uint64_t data_mbytes = 0;

double sinkStart = 0;
double appStart = 1;
double stopTime = 20;

double start_time = 0;
double duration = 100.0;
double stop_time;

bool sack = true;

std::string recovery = "ns3::TcpClassicRecovery";
std::string filePrefix;
Ptr<LrWpanErrorModel> lrWpanError;


uint numPackets = 200;
uint packetSize = 1024;
std::string transferSpeed = "1Mbps"; //App dataRate
//===================================================




int main(int argc, char **argv)
{
    //===========================CONFIG SETUP=========================================

    //changing default Congestion Control Algo by adding this line:
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpWestwood"));
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
                       TypeIdValue(TypeId::LookupByName(recovery)));


    // WHY?
    //  2 MB of TCP buffer
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1 << 21));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1 << 21));
    Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(sack));

    // WHAT?
    Header *temp_header = new Ipv6Header();
    uint32_t ip_header = temp_header->GetSerializedSize();
    delete temp_header;
    temp_header = new TcpHeader();
    uint32_t tcp_header = temp_header->GetSerializedSize();
    delete temp_header;
    tcp_adu_size = mtu_bytes - 20 - (ip_header + tcp_header);

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(tcp_adu_size));
    //===============================================================

    stop_time = start_time + duration;

    lrWpanError = CreateObject<LrWpanErrorModel>();

    std::cout << "------------------------------------------------------\n";
    std::cout << "Source Count: " << n_wirelessNodes << "\n";
    std::cout << "------------------------------------------------------\n";

    Packet::EnablePrinting();


    //=========making the Nodes====================================
    n_wireless_nd_ap = n_wirelessNodes + 1; //1 extra AP Node conn. to CSMA
    NodeContainer wsnNodes;
    wsnNodes.Create(n_wireless_nd_ap);

    NodeContainer wiredNodes;
    wiredNodes.Create(1);
    wiredNodes.Add(wsnNodes.Get(0));
    //=============================================


  
    //=========setting up mobility=================================
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0),
                                  "MinY", DoubleValue(0),
                                  "DeltaX", DoubleValue(1),
                                  "DeltaY", DoubleValue(1),
                                  "GridWidth", UintegerValue(4),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wsnNodes);
    //============================================================


    //===============Installing the Netdevices==========================
    LrWpanHelper lrWpanHelper;
    NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(wsnNodes);

    lrWpanHelper.AssociateToPan(lrwpanDevices, 0);

    InternetStackHelper internetv6;
    internetv6.Install(wsnNodes);
    internetv6.Install(wiredNodes.Get(0));

    SixLowPanHelper sixLowPanHelper;
    NetDeviceContainer sixLowPanDevices = sixLowPanHelper.Install(lrwpanDevices);

    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute ("DataRate", StringValue ("20Mbps"));
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
    for (uint32_t i = 1; i <= n_wirelessNodes; i++)
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
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("pan.cwnd");
    ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
    //===================================================================




    //============Flow Mon init===============
    FlowMonitorHelper flowHelper;
    flowHelper.InstallAll();


    //============Simulator init====================
    Simulator::Stop(Seconds(stop_time));
    Simulator::Run();


    //==============Flow Monitor Calculations=============================






    //=====================================================================

    Simulator::Destroy();

    return 0;
}
