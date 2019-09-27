#include <memory>
#include <iostream>
#include <boost/asio.hpp>
#include "AwaitingHost.h"
#include "Bridge.h"

void AwaitingHost::remove(const std::string& name) {
    awaiting_hosts_.erase(name);
    std::cerr << name << " removed from list\n";
}

std::shared_ptr<Bridge> AwaitingHost::match_host(const std::string& name, std::shared_ptr<ClientConnection> client) {
    auto iter = awaiting_hosts_.find(name);
    if (iter == awaiting_hosts_.end()) return nullptr;
    if (auto host = iter->second.lock()) {
        return std::make_shared<Bridge>(host, client);
    }
    return nullptr;
}

bool AwaitingHost::enlist(std::shared_ptr<HostConnection> host) {
    auto iter = awaiting_hosts_.find(host->get_name());
    if (iter == awaiting_hosts_.end()) {
        awaiting_hosts_.emplace(host->get_name(), host);
        return true;
    }
    return false;
}

std::vector<std::string> AwaitingHost::waiting_hosts() {
    std::vector<std::string> results;
    for (auto& iter : awaiting_hosts_) {
        results.push_back(iter.first);
    }
    return results;
}
