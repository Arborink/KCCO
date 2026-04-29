#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

#include "transport_api.h"

#ifdef _WIN32
#ifdef OVERLAY_BUILD
#define OVERLAY_API __declspec(dllexport)
#else
#define OVERLAY_API __declspec(dllimport)
#endif
#else
#define OVERLAY_API __attribute__((visibility("default")))
#endif

struct ResourceEntry
{
    std::string key;
    std::string value;
};

struct ResourceResult
{
    uint64_t    node_id;
    std::string key;
    std::string value;
    uint64_t    query_id;   // ★ 新增，對應 query_resource 的 query_id
};

struct OverlayStats
{
    uint64_t messages_sent     = 0;
    uint64_t messages_received = 0;
};

/*
 Node event type
*/
enum class NodeEvent
{
    JOINED,
    LEFT,
    FAILED
};

struct OverlayConfig
{
    uint64_t node_id;
    int k                  = 3;
    int gateway_limit      = 3;
    int gossip_interval    = 1000;
    int heartbeat_interval = 2000;
    double   max_bandwidth_mbps = 1000.0;  // ★ 新增
    bool     has_battery        = true;   // ★ 新增
};

class OVERLAY_API OverlayNode
{
public:
    virtual ~OverlayNode() = default;

    virtual void start() = 0;
    virtual void stop()  = 0;

    virtual bool join(
        const std::vector<uint64_t>& bootstrap_nodes
    ) = 0;

    virtual void leave() = 0;

/*
 Publish a resource to the overlay network.
 
 key:   A string identifying the resource.
        Recommended format: "<type>:<identifier>"
        Examples:
          "service:video-streaming"   — service discovery
          "file:sha256:<hash>"        — content addressing
          "user:alice"                — identity lookup
          "device:sensor-001"         — device registry
 
 value: Arbitrary string associated with the key.
        Examples: address, metadata, JSON, base64 data.
 
 Notes:
  - Publishing the same key again updates the value.
  - Keys are case-sensitive.
  - No enforced format — convention is up to the application.
*/
    virtual void publish_resource(
        const ResourceEntry& resource
    ) = 0;

    /*
     Publish multiple resources in one call
    */
    virtual void publish_resources(
        const std::vector<ResourceEntry>& resources
    ) = 0;

    virtual void remove_resource(
        const std::string& key
    ) = 0;

    /*
     Query resource
     query_id: application-defined id, echoed back in ResourceResult
     Routing: local -> clique center -> cross-clique
    */
    virtual void query_resource(
        const std::string& key,
        uint64_t           query_id = 0
    ) = 0;

    virtual void register_query_callback(
        std::function<void(const ResourceResult&)> handler
    ) = 0;

    /*
     Node join/leave/failure event callback
     handler(node_id, event)
    */
    virtual void register_node_event_callback(
        std::function<void(uint64_t, NodeEvent)> handler
    ) = 0;

    virtual uint64_t get_node_id() const = 0;

    virtual uint64_t get_clique_id() const = 0;

    virtual std::vector<uint64_t> get_centers() const = 0;

    virtual std::vector<uint64_t> get_gateways() const = 0;

    virtual std::vector<uint64_t> get_neighbors() const = 0;

    virtual std::vector<uint64_t> get_clique_members() const = 0;

    virtual OverlayStats get_stats() const = 0;

    virtual void ping(uint64_t target_node_id) = 0;

    /*
    For testing and debugging only.
    Force this node to start a new clique without searching.
    Not intended for production use.
    */
    virtual void force_new_clique() = 0;
};

OVERLAY_API std::shared_ptr<OverlayNode> create_overlay_node(
    const OverlayConfig& config,
    std::shared_ptr<TransportAPI> transport
);
