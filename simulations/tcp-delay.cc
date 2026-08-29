#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

// --------------------------------------------------
// Global variables
// --------------------------------------------------

uint64_t totalReceivedBytes = 0;
uint64_t totalReceivedPackets = 0;

double totalDelay = 0.0;


// --------------------------------------------------
// TCP receive callback
// --------------------------------------------------

void PacketReceivedCallback(
    Ptr<const Packet> packet,
    const Address &from)
{
    totalReceivedBytes += packet->GetSize();
    totalReceivedPackets++;
}


// --------------------------------------------------
// Main
// --------------------------------------------------

int main(int argc, char *argv[])
{
    // --------------------------------------------------
    // 1. Command-line configuration
    // --------------------------------------------------

    CommandLine cmd;

    std::string dataRate = "1Mbps";

    cmd.AddValue(
        "rate",
        "TCP offered data rate",
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
    // 3. Create two nodes
    // --------------------------------------------------

    NodeContainer nodes;

    nodes.Create(2);


    // --------------------------------------------------
    // 4. Create point-to-point link
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
    // 5. Install network devices
    // --------------------------------------------------

    NetDeviceContainer devices =
        pointToPoint.Install(nodes);


    // --------------------------------------------------
    // 6. Install Internet stack
    // --------------------------------------------------

    InternetStackHelper internet;

    internet.Install(nodes);


    // --------------------------------------------------
    // 7. Assign IP addresses
    // --------------------------------------------------

    Ipv4AddressHelper address;

    address.SetBase(
        "10.1.1.0",
        "255.255.255.0"
    );

    Ipv4InterfaceContainer interfaces =
        address.Assign(devices);


    // --------------------------------------------------
    // 8. TCP receiver
    // --------------------------------------------------

    uint16_t port = 8080;

    PacketSinkHelper sinkHelper(
        "ns3::TcpSocketFactory",
        InetSocketAddress(
            Ipv4Address::GetAny(),
            port
        )
    );

    ApplicationContainer sinkApp =
        sinkHelper.Install(nodes.Get(1));

    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(simulationStop));


    // --------------------------------------------------
    // 9. TCP sender
    // --------------------------------------------------

    OnOffHelper client(
        "ns3::TcpSocketFactory",
        InetSocketAddress(
            interfaces.GetAddress(1),
            port
        )
    );


    // --------------------------------------------------
    // 10. Configure TCP traffic
    // --------------------------------------------------

    client.SetAttribute(
        "DataRate",
        StringValue(dataRate)
    );

    client.SetAttribute(
        "PacketSize",
        UintegerValue(1024)
    );

    // Keep the application continuously ON.

    client.SetAttribute(
        "OnTime",
        StringValue(
            "ns3::ConstantRandomVariable[Constant=1000]"
        )
    );

    client.SetAttribute(
        "OffTime",
        StringValue(
            "ns3::ConstantRandomVariable[Constant=0]"
        )
    );


    // --------------------------------------------------
    // 11. Install TCP sender
    // --------------------------------------------------

    ApplicationContainer clientApp =
        client.Install(nodes.Get(0));

    clientApp.Start(Seconds(trafficStart));
    clientApp.Stop(Seconds(trafficStop));


    // --------------------------------------------------
    // 12. Start simulation
    // --------------------------------------------------

    std::cout << std::endl;

    std::cout << "Starting TCP simulation..."
              << std::endl;

    std::cout << "Configured offered rate: "
              << dataRate
              << std::endl;

    std::cout << "Traffic period: "
              << trafficStart
              << " - "
              << trafficStop
              << " seconds"
              << std::endl;

    std::cout << "Simulation ends at: "
              << simulationStop
              << " seconds"
              << std::endl;


    Simulator::Run();

    std::cout << "Simulation finished."
              << std::endl;


    // --------------------------------------------------
    // 13. Get PacketSink statistics
    // --------------------------------------------------

    Ptr<PacketSink> sink =
        DynamicCast<PacketSink>(
            sinkApp.Get(0)
        );

    uint64_t totalBytes =
        sink->GetTotalRx();


    // --------------------------------------------------
    // 14. Calculate throughput
    // --------------------------------------------------

    double measurementTime =
        trafficStop - trafficStart;

    double throughput =
        (totalBytes * 8.0)
        / measurementTime;

    throughput =
        throughput / 1e6;


    // --------------------------------------------------
    // 15. Display results
    // --------------------------------------------------

    std::cout << std::endl;

    std::cout << "============================="
              << std::endl;

    std::cout << "TCP SIMULATION RESULTS"
              << std::endl;

    std::cout << "============================="
              << std::endl;

    std::cout << "Node 0 IP: "
              << interfaces.GetAddress(0)
              << std::endl;

    std::cout << "Node 1 IP: "
              << interfaces.GetAddress(1)
              << std::endl;

    std::cout << "Configured offered rate: "
              << dataRate
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
    // 16. Destroy simulation
    // --------------------------------------------------

    Simulator::Destroy();

    return 0;
}