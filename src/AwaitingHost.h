#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>

#include "Connection.h"

// should be thread safe
class AwaitingHost {
public:
    static AwaitingHost& getAwaitingHosts() {
        static AwaitingHost awaitings;
        return awaitings;
    }
    void remove(const std::string& name);

    // Atomically, match with a host or error out
    // work this out later
    bool match_host(const std::string& name, std::shared_ptr<ClientConnection> client);


    // Atomically, add itself or error out
    bool enlist(std::shared_ptr<HostConnection> host);
private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<HostConnection>> awaiting_hosts_;
};

