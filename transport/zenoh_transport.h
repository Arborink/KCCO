#pragma once

#include <memory>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <zenoh.hxx>

#include "transport_api.h"

/*
 Production-grade Zenoh P2P transport
*/
class ZenohTransport : public TransportAPI
{
public:
    ZenohTransport(
        uint64_t node_id,
        const std::vector<std::string>& peers = {}
    );

    ~ZenohTransport();

    void start() override;
    void stop()  override;

    void send(
        uint64_t       dst,
        const uint8_t* data,
        size_t         size
    ) override;

    void broadcast(
        const uint8_t* data,
        size_t         size
    ) override;

    void register_receive_handler(
        const OverlayReceiver& handler
    ) override;

    uint64_t local_node_id() const override;

private:
    uint64_t                    node_id;
    std::vector<std::string>    peers;

    std::optional<zenoh::Session>                  session;
    std::optional<zenoh::Subscriber<void>>         unicast_sub;
    std::optional<zenoh::Subscriber<void>>         broadcast_sub;
    std::optional<zenoh::Publisher>                broadcast_pub;

    std::unordered_map<uint64_t,
        zenoh::Publisher> unicast_publishers;    
    std::mutex pub_mutex;

    OverlayReceiver receiver;
    std::atomic<bool> running{false};

private:
    std::string        node_key() const;
    static std::string node_key(uint64_t node);
    static std::string broadcast_key();
};
