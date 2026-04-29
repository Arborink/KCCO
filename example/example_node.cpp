#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <vector>

#include "overlay_api.h"
#include "transport_api.h"
#include "zenoh_transport.h"

static void print_help()
{
    std::cout << "\ncommands:\n";
    std::cout << "  pub <key> <value>\n";
    std::cout << "  pubs <key1> <val1> <key2> <val2> ...\n";  
    std::cout << "  query <key> [query_id]\n";
    std::cout << "  info\n";
    std::cout << "  neighbors\n";
    std::cout << "  clique\n";
    std::cout << "  stats\n";
    std::cout << "  ping <node_id>\n";
    std::cout << "  help\n";
    std::cout << "  exit\n";
    std::cout << "  startup options:\n";
    std::cout << "  --new-clique   force start a new clique (for testing only)\n";
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "usage: example_node <node_id> [bootstrap_node_ids...]\n";
        return 0;
    }

    uint64_t node_id = std::stoull(argv[1]);

    std::vector<uint64_t> bootstrap;

    bool force_new_clique = false;

    for (int i = 2; i < argc; i++)
    {
        if (std::string(argv[i]) == "--new-clique")
            force_new_clique = true;
        else
            bootstrap.push_back(std::stoull(argv[i]));
    }
   
    /*
     Create transport
    */
    auto transport = std::make_shared<ZenohTransport>(node_id);

    /*
     Configure overlay
    */
    OverlayConfig config;
    config.node_id            = node_id;
    config.k                  = 3;
    config.gateway_limit      = 3;
    config.gossip_interval    = 1000;
    config.heartbeat_interval = 2000;

    /*
     Create overlay node
    */
    auto node = create_overlay_node(config, transport);

    /*
     Register query callback
    */
    node->register_query_callback(
        [](const ResourceResult& r)
        {
            std::cout
                << "resource found:"
                << " query_id=" << r.query_id
                << " node="  << r.node_id
                << " key="   << r.key
                << " value=" << r.value
                << std::endl;
        }
    );

    /*
     Register node event callback
    */
    node->register_node_event_callback(
        [](uint64_t nid, NodeEvent event)
        {
            switch (event)
            {
            case NodeEvent::JOINED:
                std::cout << "node joined: " << nid << "\n";
                break;
            case NodeEvent::LEFT:
                std::cout << "node left: "   << nid << "\n";
                break;
            case NodeEvent::FAILED:
                std::cout << "node failed: " << nid << "\n";
                break;
            }
        }
    );

    /*
     Start system
    */
    transport->start();
    node->start();
    if (force_new_clique)
        node->force_new_clique();
    else if (!node->join(bootstrap))
    {
        std::cout << "Failed to join network, exiting\n";
        node->stop();
        transport->stop();
        return 1;
    }

    std::cout << "node " << node_id << " started\n";
    print_help();

    /*
     CLI
    */
    while (true)
    {
        std::cout << "\n> ";

        std::string line;

        if (!std::getline(std::cin, line))
            break;

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "pub")
        {
            ResourceEntry r;
            if (!(ss >> r.key >> r.value))  // ★ 檢查輸入是否完整
            {
                std::cout << "usage: pub <key> <value>\n";
            }
            else
            {
                node->publish_resource(r);
                std::cout << "resource published\n";
            }
        }
        else if (cmd == "pubs")
        {
            std::vector<ResourceEntry> entries;
            std::string k, v;
            while (ss >> k >> v)
            {
                ResourceEntry e;
                e.key   = k;
                e.value = v;
                entries.push_back(e);
            }
            node->publish_resources(entries);
            std::cout << "published " << entries.size() << " resources\n";
        }
        else if (cmd == "query")
        {
            std::string key;
            uint64_t qid = 0;
            ss >> key >> qid;
            node->query_resource(key, qid);
        }
        else if (cmd == "info")
        {
            auto centers  = node->get_centers();
            auto gateways = node->get_gateways();

            std::cout << "clique: "
                      << node->get_clique_id() << "\n";

            std::cout << "centers: ";
            for (auto c : centers)
                std::cout << c << " ";
            std::cout << "\n";

            std::cout << "gateways: ";
            for (auto g : gateways)
                std::cout << g << " ";
            std::cout << "\n";
        }
        else if (cmd == "neighbors")
        {
            auto neighbors = node->get_neighbors();
            std::cout << "neighbors: ";
            for (auto n : neighbors)
                std::cout << n << " ";
            std::cout << "\n";
        }
        else if (cmd == "clique")
        {
            auto members = node->get_clique_members();
            std::cout << "clique members: ";
            for (auto m : members)
                std::cout << m << " ";
            std::cout << "\n";
        }
        else if (cmd == "stats")
        {
            auto stats = node->get_stats();
            std::cout << "messages sent:     "
                      << stats.messages_sent     << "\n";
            std::cout << "messages received: "
                      << stats.messages_received << "\n";
        }
        else if (cmd == "ping")
        {
            uint64_t target;
            ss >> target;
            node->ping(target);
            std::cout << "ping sent to node " << target
                      << " (RTT will print on reply)\n";
        }
        else if (cmd == "help")
        {
            print_help();
        }
        else if (cmd == "exit")
        {
            break;
        }
        else if (cmd.empty())
        {
            continue;
        }
        else
        {
            std::cout << "unknown command\n";
        }
    }

    /*
     Shutdown
    */
    node->leave();
    node->stop();
    transport->stop();

    return 0;
}
