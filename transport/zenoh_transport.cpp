#include "zenoh_transport.h"
#include <iostream>

ZenohTransport::ZenohTransport(
    uint64_t id,
    const std::vector<std::string>& p)
: node_id(id), peers(p)
{
}

ZenohTransport::~ZenohTransport()
{
    stop();
}

void ZenohTransport::start()
{
    if (running)
        return;

    running = true;

    auto config = zenoh::Config::create_default();

    /*
     peer mode
    */
    config.insert_json5(
        "mode",
        "\"peer\""
    );

    /*
     bootstrap peers
    */
    if (!peers.empty())
    {
        std::string peer_list = "[";

        for (size_t i = 0; i < peers.size(); i++)
        {
            peer_list += "\"" + peers[i] + "\"";

            if (i + 1 < peers.size())
                peer_list += ",";
        }

        peer_list += "]";

        config.insert_json5(
            "connect/endpoints",
            peer_list
        );
    }

    session = zenoh::Session::open(std::move(config));

    /*
     unicast subscriber
    */
    unicast_sub = session->declare_subscriber(
        zenoh::KeyExpr(node_key()),
        [this](const zenoh::Sample& sample)
        {
            auto payload = sample.get_payload().as_vector();

            if (receiver)
                receiver(payload.data(), payload.size());
        },
        zenoh::closures::none
    );

    /*
     broadcast subscriber
    */
    broadcast_sub = session->declare_subscriber(
        zenoh::KeyExpr(broadcast_key()),
        [this](const zenoh::Sample& sample)
        {
            auto payload = sample.get_payload().as_vector();

            if (receiver)
                receiver(payload.data(), payload.size());
        },
        zenoh::closures::none
    );

    /*
     broadcast publisher
    */
    broadcast_pub = session->declare_publisher(
        zenoh::KeyExpr(broadcast_key())
    );

    std::cout
        << "Zenoh P2P transport started node "
        << node_id
        << std::endl;
}

void ZenohTransport::stop()
{
    if (!running)
        return;

    running = false;

    unicast_publishers.clear();

    unicast_sub.reset();
    broadcast_sub.reset();
    broadcast_pub.reset();
    session.reset();
}

void ZenohTransport::send(
    uint64_t       dst,
    const uint8_t* data,
    size_t         size)
{
    if (!session)
        return;

    zenoh::Publisher* pub = nullptr;

    {
        std::lock_guard<std::mutex> lock(pub_mutex);

        auto it = unicast_publishers.find(dst);

        if (it == unicast_publishers.end())
        {
            auto result = unicast_publishers.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(dst),
                std::forward_as_tuple(
                    session->declare_publisher(
                        zenoh::KeyExpr(node_key(dst))
                    )
                )
            );

            pub = &result.first->second;
        }
        else
        {
            pub = &it->second;
        }
    }

    pub->put(zenoh::Bytes(
        std::vector<uint8_t>(data, data + size)
    ));
}

void ZenohTransport::broadcast(
    const uint8_t* data,
    size_t         size)
{
    if (!broadcast_pub)
        return;

    broadcast_pub->put(zenoh::Bytes(
        std::vector<uint8_t>(data, data + size)
    ));
}

void ZenohTransport::register_receive_handler(
    const OverlayReceiver& handler)
{
    receiver = handler;
}

uint64_t ZenohTransport::local_node_id() const
{
    return node_id;
}

/*
 key helpers
*/

std::string ZenohTransport::node_key() const
{
    return "overlay/node/" + std::to_string(node_id);
}

std::string ZenohTransport::node_key(uint64_t node)
{
    return "overlay/node/" + std::to_string(node);
}

std::string ZenohTransport::broadcast_key()
{
    return "overlay/broadcast";
}