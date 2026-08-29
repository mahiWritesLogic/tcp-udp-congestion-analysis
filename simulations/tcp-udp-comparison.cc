#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/applications-module.h"
#include "ns3/packet.h"

using namespace ns3;
class TimestampTag : public Tag
{
public:
    static TypeId GetTypeId()
    {
        static TypeId tid =
            TypeId("TimestampTag")
            .SetParent<Tag>()
            .AddConstructor<TimestampTag>();

        return tid;
    }

    TypeId GetInstanceTypeId() const override
    {
        return GetTypeId();
    }

    uint32_t GetSerializedSize() const override
    {
        return sizeof(uint64_t);
    }

    void Serialize(TagBuffer i) const override
    {
        i.WriteU64(m_timestamp);
    }

    void Deserialize(TagBuffer i) override
    {
        m_timestamp = i.ReadU64();
    }

    void Print(std::ostream &os) const override
    {
        os << m_timestamp;
    }

    void SetTimestamp(uint64_t timestamp)
    {
        m_timestamp = timestamp;
    }

    uint64_t GetTimestamp() const
    {
        return m_timestamp;
    }

private:
    uint64_t m_timestamp = 0;
};

// Global counter for transmitted packets
uint32_t packetsSent = 0;

// Function called whenever the UDP sender transmits a packet
void PacketSentCallback(Ptr<const Packet> packet)
{
    packetsSent++;
}

int main()
{
    // --------------------------------------------------
    // 1. Create two nodes
    // --------------------------------------------------

    NodeContainer nodes;
    nodes.Create(2);


    // --------------------------------------------------
    // 2. Create point-to-point link
    // --------------------------------------------------

    PointToPointHelper pointToPoint;

    pointToPoint.SetDeviceAttribute(
        "DataRate",
        StringValue("5Mbps")
    );

    pointToPoint.SetChannelAttribute(
        "Delay",
        StringValue("2ms")
    );


    // --------------------------------------------------
    // 3. Install network devices
    // --------------------------------------------------

    NetDeviceContainer devices;

    devices = pointToPoint.Install(nodes);


    // --------------------------------------------------
    // 4. Install Internet stack
    // --------------------------------------------------

    InternetStackHelper internet;

    internet.Install(nodes);


    // --------------------------------------------------
    // 5. Assign IP addresses
    // --------------------------------------------------

    Ipv4AddressHelper address;

    address.SetBase(
        "10.1.1.0",
        "255.255.255.0"
    );

    Ipv4InterfaceContainer interfaces;

    interfaces = address.Assign(devices);


    // --------------------------------------------------
    // 6. Create UDP receiver
    // --------------------------------------------------

    uint16_t port = 8080;

    PacketSinkHelper sinkHelper(
        "ns3::UdpSocketFactory",
        InetSocketAddress(
            Ipv4Address::GetAny(),
            port
        )
    );

    ApplicationContainer sinkApp =
        sinkHelper.Install(nodes.Get(1));

    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(10.0));


    // --------------------------------------------------
    // 7. Create UDP sender
    // --------------------------------------------------

    OnOffHelper client(
        "ns3::UdpSocketFactory",
        InetSocketAddress(
            interfaces.GetAddress(1),
            port
        )
    );


    // --------------------------------------------------
    // 8. Configure UDP traffic
    // --------------------------------------------------

    client.SetAttribute(
        "DataRate",
        StringValue("1Mbps")
    );

    client.SetAttribute(
        "PacketSize",
        UintegerValue(1024)
    );


    // --------------------------------------------------
    // 9. Install sender on Node 0
    // --------------------------------------------------

    ApplicationContainer clientApp =
        client.Install(nodes.Get(0));

    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));


    // --------------------------------------------------
    // 10. Connect to sender's Tx trace
    // --------------------------------------------------

    clientApp.Get(0)->TraceConnectWithoutContext(
        "Tx",
        MakeCallback(&PacketSentCallback)
    );


    // --------------------------------------------------
    // 11. Start simulation
    // --------------------------------------------------

    std::cout << "Starting simulation..."
              << std::endl;

    Simulator::Run();


    // --------------------------------------------------
    // 12. Get received bytes
    // --------------------------------------------------

    Ptr<PacketSink> sink =
        DynamicCast<PacketSink>(sinkApp.Get(0));

    uint64_t totalBytes =
        sink->GetTotalRx();


    // --------------------------------------------------
    // 13. Calculate received packets
    // --------------------------------------------------

    uint32_t packetSize = 1024;

    uint32_t packetsReceived =
        totalBytes / packetSize;


    // --------------------------------------------------
    // 14. Calculate packet loss
    // --------------------------------------------------

    uint32_t packetsLost =
        packetsSent - packetsReceived;

    double packetLoss = 0.0;

    if (packetsSent > 0)
    {
        packetLoss =
            (static_cast<double>(packetsLost)
             / packetsSent) * 100.0;
    }


    // --------------------------------------------------
    // 15. Calculate throughput
    // --------------------------------------------------

    double simulationTime = 8.0;

    double throughput =
        (totalBytes * 8.0)
        / simulationTime;

    throughput =
        throughput / 1e6;


    // --------------------------------------------------
    // 16. Display results
    // --------------------------------------------------

    std::cout << std::endl;

    std::cout << "============================="
              << std::endl;

    std::cout << "UDP SIMULATION RESULTS"
              << std::endl;

    std::cout << "============================="
              << std::endl;

    std::cout << "Node 0 IP: "
              << interfaces.GetAddress(0)
              << std::endl;

    std::cout << "Node 1 IP: "
              << interfaces.GetAddress(1)
              << std::endl;

    std::cout << "Packet size: "
              << packetSize
              << " bytes"
              << std::endl;

    std::cout << "Packets sent: "
              << packetsSent
              << std::endl;

    std::cout << "Packets received: "
              << packetsReceived
              << std::endl;

    std::cout << "Packets lost: "
              << packetsLost
              << std::endl;

    std::cout << "Packet loss: "
              << packetLoss
              << " %"
              << std::endl;

    std::cout << "Total received bytes: "
              << totalBytes
              << std::endl;

    std::cout << "Throughput: "
              << throughput
              << " Mbps"
              << std::endl;

    std::cout << "============================="
              << std::endl;


    // --------------------------------------------------
    // 17. Destroy simulation
    // --------------------------------------------------

    Simulator::Destroy();

    return 0;
}