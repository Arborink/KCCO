#pragma once

#include <functional>
#include <cstdint>
#include <cstddef>

/*
Transport abstraction layer
Overlay core (.so) does not depend on any
network stack. All networking is implemented
through this interface.
*/

using OverlayReceiver =
std::function<void(const uint8_t* data, size_t size)>;

class TransportAPI
{
public:
    virtual ~TransportAPI() = default;

    /*
     Send raw packet to destination node
    */
    virtual void send(
        uint64_t dst,
        const uint8_t* data,
        size_t size
    ) = 0;

    /*
     Broadcast packet
     Used by gossip / discovery
    */
    virtual void broadcast(
        const uint8_t* data,
        size_t size
    ) = 0;

    /*
     Register receive handler
    */
    virtual void register_receive_handler(
        const OverlayReceiver& handler
    ) = 0;

    /*
     Start transport runtime
    */
    virtual void start() = 0;

    /*
     Stop transport runtime
    */
    virtual void stop() = 0;

    /*
     Return local node id
    */
    virtual uint64_t local_node_id() const = 0;
};
