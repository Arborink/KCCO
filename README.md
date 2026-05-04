# KCCO Overlay

KCCO (K-Clique Community Overlay) is a mobile P2P overlay networking framework designed for scalable decentralized resource discovery under churn.

The system organizes nodes into bounded-size communities and maintains an efficient distributed resource index without relying on centralized infrastructure.

# Overlay Network SDK

A lightweight mobile P2P overlay networking SDK designed for scalable resource discovery and decentralized service coordination.

This repository provides the public SDK interface and example integrations.  
The core overlay algorithms are distributed as a precompiled shared library.

---

# Overview

The Overlay Network SDK enables applications to form a self-organizing distributed network without relying on centralized servers.

Typical use cases include:

- decentralized service discovery
- distributed resource indexing
- peer-to-peer coordination
- edge computing networks
- IoT service routing

The SDK provides a simple API for joining the overlay network, publishing resources, and querying distributed indexes.

The internal algorithms are implemented inside a binary library and protected by an authorized patent.

---

# Architecture

The system is designed with a transport-independent architecture.

Application
↓
Overlay API
↓
Overlay Core (liboverlay.so)
↓
Transport Adapter
↓
Network (Zenoh / TCP / others)


The core overlay logic does not depend on any specific network protocol.

Applications can integrate different transports by implementing the provided transport interface.

---

# Repository Structure


overlay-sdk
│
├── include
│ ├── overlay_api.h
│ └── transport_api.h
│
├── transport
│ └── zenoh_transport.cpp
│
├── examples
│ └── example_node.cpp
│
└── lib
  └── liboverlay.so


Description:

| Directory | Description |
|--------|--------|
| include | Public SDK headers |
| transport | Example transport adapter |
| examples | Example applications |
| lib | Precompiled overlay core library |

---

# Requirements

- Linux (x86_64)
- C++17 or newer
- Zenoh runtime (for the provided transport example)

Example transport implementation uses **Zenoh**:

Software: :contentReference[oaicite:1]{index=1}

---

## Installation

1. Download "liboverlay.so" from [Releases](https://github.com/arborink/KCCO/releases)
2. Place it in the "lib/"directory
3. Build:
bash
cmake -B build
cmake --build build

# Build Example

Example build command:


g++ examples/example_node.cpp
transport/zenoh_transport.cpp
-Iinclude
-Llib -loverlay
-lzenohc
-std=c++17
-o example_node


Adjust the Zenoh library path depending on your installation.

---

# Running Example

Start the first node:


./example_node 1


Start additional nodes:


./example_node 2 [1]
./example_node 3 [1]


The second parameter specifies a bootstrap node, which is optional.


---

# Example Commands

Publish a resource:


pub camera rtsp://10.0.0.5


Query a resource:


query camera


Show node information:


info


---

# SDK Usage

Basic usage:

auto transport = std::make_shared<ZenohTransport>();

OverlayConfig config;
config.node_id = 1;

auto node = create_overlay_node(config, transport);

transport->start();
node->start();

node->join({2,3});

ResourceEntry r;
r.key = "camera";
r.value = "rtsp://10.0.0.5";

node->publish_resource(r);

Applications interact only with the Overlay API.

The internal network algorithms remain inside the binary library.

Transport Integration

The SDK defines a transport abstraction:


TransportAPI


Developers can integrate alternative network layers such as:

TCP
UDP
QUIC
custom messaging systems

A reference adapter based on Zenoh is provided in this repository.

## Simulation Results (ns-3)

Performance comparison between KCCO overlay and Zenoh-over-DDS hybrid mode,
simulated 1000 mobile nodes using ns-3 . 

| Metric | KCCO | Zenoh-over-DDS |
|--------|------|----------------|
| Resource Discovery Success Rate | **76%** | 63.6% |
| Average Discovery Latency | **1.99 s** | 9.99 s |
| Physical Layer Collisions | **459,316** | 2,076,111 |
| Control Plane Overhead (Queries) | **14,918** | 47,267 |
| Metadata Volume | **7.28 MB** | 23.08 MB |

KCCO outperforms Zenoh-over-DDS across all metrics:
- **20% higher** resource discovery success rate
- **5x lower** discovery latency
- **4.5x fewer** physical layer collisions
- **3.2x lower** control plane overhead
- **3.2x less** metadata volume

## Live Demo

[View the interactive demo](https://arborink.github.io/KCCO/demo/robot_demo.html)

License

The SDK interface and example code are provided for integration and evaluation.

The overlay core implementation contained in liboverlay.so is protected by authorized patents and distributed in binary form.

Contributing

Contributions are welcome for:

transport adapters
example applications
tooling and documentation

Please note that modifications to the overlay core algorithms are not part of this repository.

Disclaimer

This project provides a software development kit for building distributed applications.
The internal algorithms are implemented in a binary library and may be subject to intellectual property protection.
