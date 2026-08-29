#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

// --------------------------------------------------
// Global measurement variables
// --------------------------------------------------

uint64_t measuredPacketsReceived = 0;

uint64_t measuredBytesReceived = 0;

uint64_t totalPacketsReceived = 0;

uint64_t totalPacketsLost = 0;

double totalDelay = 0.0;


// --------------------------------------------------
// Measurement window
// --------------------------------------------------

double measurementStart = 2.0;

double measurementStop = 10.0;


// --------------------------------------------------
// UDP receive callback
// --------------------------------------------------

void PacketReceivedCallback(
    Ptr<const Packet> packet,
    const Address &from,
    const Address &local)
{
    // Always count total received packets
    totalPacketsReceived++;

    // Current simulation time
    double now =
        Simulator::Now().GetSeconds();


    // --------------------------------------------------
    // Only measure packets arriving during
    // the actual traffic window
    // --------------------------------------------------

    if (now >= measurementStart &&
        now <= measurementStop)
    {
        // Make a copy
        Ptr<Packet> copy =
            packet->Copy();

        // Extract timestamp
        SeqTsHeader timestampHeader;

        copy->RemoveHeader(
            timestampHeader
        );

        // Transmission time
        Time transmitTime =
            timestampHeader.GetTs();

        // Reception time
        Time receiveTime =
            Simulator::Now();

        // Delay
        Time delay =
            receiveTime - transmitTime;

        // Store measurements
        totalDelay +=
            delay.GetSeconds();

        measuredPacketsReceived++;

        measuredBytesReceived +=
            packet->GetSize();
    }
}


int main(int argc, char *argv[])
{
    // --------------------------------------------------
    // 1. Command-line configuration
    // --------------------------------------------------

    CommandLine cmd;

    std::string dataRate = "1Mbps";

    cmd.AddValue(
        "rate",
        "UDP offered data rate",
        dataRate
    );

    cmd.Parse(argc, argv);


    // --------------------------------------------------
    // 2. Simulation timing
    // --------------------------------------------------

    double trafficStart = 2.0;

    double trafficStop = 10.0;

    double simulationStop = 15.0;


    // --------------------------------------------------
    // 3. Packet configuration
    // --------------------------------------------------

    uint32_t packetSize = 1024;

    DataRate rate(dataRate);

    // Calculate packet interval
    double intervalSeconds =
        (packetSize * 8.0)
        / rate.GetBitRate();

    Time packetInterval =
        Seconds(intervalSeconds);


    // --------------------------------------------------
    // 4. Calculate number of packets
    // --------------------------------------------------

    double trafficDuration =
        trafficStop - trafficStart;

    uint32_t maxPackets =
        static_cast<uint32_t>(
            trafficDuration /
            intervalSeconds
        ) + 1;


    // --------------------------------------------------
    // 5. Create nodes
    // --------------------------------------------------

    NodeContainer nodes;

    nodes.Create(2);


    // --------------------------------------------------
    // 6. Create point-to-point link
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
    // 7. Install devices
    // --------------------------------------------------

    NetDeviceContainer devices =
        pointToPoint.Install(nodes);


    // --------------------------------------------------
    // 8. Install Internet stack
    // --------------------------------------------------

    InternetStackHelper internet;

    internet.Install(nodes);


    // --------------------------------------------------
    // 9. Assign IP addresses
    // --------------------------------------------------

    Ipv4AddressHelper address;

    address.SetBase(
        "10.1.1.0",
        "255.255.255.0"
    );

    Ipv4InterfaceContainer interfaces =
        address.Assign(devices);


    // --------------------------------------------------
    // 10. Create UDP server
    // --------------------------------------------------

    uint16_t port = 8080;

    UdpServerHelper server(port);

    ApplicationContainer serverApp =
        server.Install(nodes.Get(1));

    serverApp.Start(
        Seconds(1.0)
    );

    serverApp.Stop(
        Seconds(simulationStop)
    );


    // --------------------------------------------------
    // 11. Create UDP client
    // --------------------------------------------------

    UdpClientHelper client(
        interfaces.GetAddress(1),
        port
    );


    // --------------------------------------------------
    // 12. Configure UDP client
    // --------------------------------------------------

    client.SetAttribute(
        "MaxPackets",
        UintegerValue(maxPackets)
    );

    client.SetAttribute(
        "Interval",
        TimeValue(packetInterval)
    );

    client.SetAttribute(
        "PacketSize",
        UintegerValue(packetSize)
    );


    // --------------------------------------------------
    // 13. Install UDP client
    // --------------------------------------------------

    ApplicationContainer clientApp =
        client.Install(nodes.Get(0));

    clientApp.Start(
        Seconds(trafficStart)
    );

    clientApp.Stop(
        Seconds(trafficStop)
    );


    // --------------------------------------------------
    // 14. Connect receive callback
    // --------------------------------------------------

    Ptr<UdpServer> serverPtr =
        DynamicCast<UdpServer>(
            serverApp.Get(0)
        );

    serverPtr->TraceConnectWithoutContext(
        "RxWithAddresses",
        MakeCallback(
            &PacketReceivedCallback
        )
    );


    // --------------------------------------------------
    // 15. Start simulation
    // --------------------------------------------------

    std::cout << std::endl;

    std::cout
        << "Starting UDP simulation..."
        << std::endl;

    std::cout
        << "Configured offered rate: "
        << dataRate
        << std::endl;

    std::cout
        << "Packet size: "
        << packetSize
        << " bytes"
        << std::endl;

    std::cout
        << "Packet interval: "
        << packetInterval.GetSeconds() * 1000
        << " ms"
        << std::endl;

    std::cout
        << "Traffic period: "
        << trafficStart
        << " - "
        << trafficStop
        << " seconds"
        << std::endl;

    std::cout
        << "Measurement period: "
        << measurementStart
        << " - "
        << measurementStop
        << " seconds"
        << std::endl;

    std::cout
        << "Drain period: "
        << trafficStop
        << " - "
        << simulationStop
        << " seconds"
        << std::endl;


    Simulator::Stop(
        Seconds(simulationStop)
    );

    Simulator::Run();


    std::cout
        << "Simulation finished."
        << std::endl;


    // --------------------------------------------------
    // 16. Get server packet statistics
    // --------------------------------------------------

    uint64_t packetsReceived =
        serverPtr->GetReceived();

    uint32_t packetsLost =
        serverPtr->GetLost();


    // --------------------------------------------------
    // 17. Total packets sent
    // --------------------------------------------------

    uint64_t packetsSent =
        packetsReceived +
        packetsLost;


    // --------------------------------------------------
    // 18. Calculate packet loss
    // --------------------------------------------------

    double packetLoss = 0.0;

    if (packetsSent > 0)
    {
        packetLoss =
            (
                static_cast<double>(
                    packetsLost
                )
                /
                packetsSent
            )
            * 100.0;
    }


    // --------------------------------------------------
    // 19. Calculate average delay
    // --------------------------------------------------

    double averageDelay = 0.0;

    if (measuredPacketsReceived > 0)
    {
        averageDelay =
            (
                totalDelay
                /
                measuredPacketsReceived
            )
            * 1000.0;
    }


    // --------------------------------------------------
    // 20. Calculate throughput
    // --------------------------------------------------

    double throughput =
        (
            measuredBytesReceived
            * 8.0
        )
        /
        (measurementStop -
         measurementStart);

    throughput =
        throughput / 1e6;


    // --------------------------------------------------
    // 21. Display results
    // --------------------------------------------------

    std::cout << std::endl;

    std::cout
        << "============================="
        << std::endl;

    std::cout
        << "UDP SIMULATION RESULTS"
        << std::endl;

    std::cout
        << "============================="
        << std::endl;

    std::cout
        << "Node 0 IP: "
        << interfaces.GetAddress(0)
        << std::endl;

    std::cout
        << "Node 1 IP: "
        << interfaces.GetAddress(1)
        << std::endl;

    std::cout
        << "Configured offered rate: "
        << dataRate
        << std::endl;

    std::cout
        << "Packets sent: "
        << packetsSent
        << std::endl;

    std::cout
        << "Packets received: "
        << packetsReceived
        << std::endl;

    std::cout
        << "Packets lost: "
        << packetsLost
        << std::endl;

    std::cout
        << "Packet loss: "
        << packetLoss
        << " %"
        << std::endl;

    std::cout
        << "Measured packets received: "
        << measuredPacketsReceived
        << std::endl;

    std::cout
        << "Average delay: "
        << averageDelay
        << " ms"
        << std::endl;

    std::cout
        << "Measured received bytes: "
        << measuredBytesReceived
        << std::endl;

    std::cout
        << "Throughput: "
        << throughput
        << " Mbps"
        << std::endl;

    std::cout
        << "============================="
        << std::endl;


    // --------------------------------------------------
    // 22. Destroy simulation
    // --------------------------------------------------

    Simulator::Destroy();

    return 0;
}