import matplotlib.pyplot as plt

# ---------------------------------------
# Experimental data
# ---------------------------------------

offered_load = [1, 3, 5, 8, 10]

tcp_throughput = [
    0.999424,
    2.99827,
    4.66534,
    4.66534,
    4.66637
]

udp_throughput = [
    1.00045,
    2.99930,
    4.85581,
    4.85581,
    4.85581
]


# ---------------------------------------
# Graph 1: Throughput
# ---------------------------------------

plt.figure(figsize=(8, 5))

plt.plot(
    offered_load,
    tcp_throughput,
    marker="o",
    label="TCP"
)

plt.plot(
    offered_load,
    udp_throughput,
    marker="s",
    label="UDP"
)

plt.xlabel("Offered Load (Mbps)")
plt.ylabel("Throughput (Mbps)")

plt.title(
    "TCP vs UDP Throughput"
)

plt.grid(True)

plt.legend()

plt.tight_layout()

plt.savefig(
    "throughput_comparison.png",
    dpi=300
)

plt.show()


# ---------------------------------------
# UDP packet loss
# ---------------------------------------

udp_packet_loss = [
    0,
    0,
    0.696436,
    27.0676,
    31.1628
]

plt.figure(figsize=(8, 5))

plt.plot(
    offered_load,
    udp_packet_loss,
    marker="o"
)

plt.xlabel("Offered Load (Mbps)")
plt.ylabel("Packet Loss (%)")

plt.title(
    "UDP Packet Loss vs Offered Load"
)

plt.grid(True)

plt.tight_layout()

plt.savefig(
    "udp_packet_loss.png",
    dpi=300
)

plt.show()


# ---------------------------------------
# UDP delay
# ---------------------------------------

udp_delay = [
    3.6864,
    3.6864,
    111.458,
    1101.01,
    1665.29
]

plt.figure(figsize=(8, 5))

plt.plot(
    offered_load,
    udp_delay,
    marker="o"
)

plt.xlabel("Offered Load (Mbps)")
plt.ylabel("Average Delay (ms)")

plt.title(
    "UDP Average Delay vs Offered Load"
)

plt.grid(True)

plt.tight_layout()

plt.savefig(
    "udp_delay.png",
    dpi=300
)

plt.show()
