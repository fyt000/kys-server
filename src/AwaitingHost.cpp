#include <memory>
#include <iostream>
#include <boost/asio.hpp>
#include "AwaitingHost.h"
#include "Bridge.h"

void AwaitingHost::remove(const std::string& name) {
    // std::lock_guard<std::mutex> guard(mutex_);
    awaiting_hosts_.erase(name);
}

std::shared_ptr<Bridge> AwaitingHost::match_host(const std::string& name, std::shared_ptr<ClientConnection> client) {
    // std::lock_guard<std::mutex> guard(mutex_);
    auto iter = awaiting_hosts_.find(name);
    if (iter == awaiting_hosts_.end()) return nullptr;
    auto bridge = std::make_shared<Bridge>(iter->second, client);
    awaiting_hosts_.erase(iter);
    return bridge;
}

bool AwaitingHost::enlist(std::shared_ptr<HostConnection> host) {
    // std::lock_guard<std::mutex> guard(mutex_);
    auto iter = awaiting_hosts_.find(host->get_name());
    int result = 1;
    if (iter != awaiting_hosts_.end()) result = 0;
   
    try {
        boost::asio::write(host->socket, boost::asio::buffer(&result, sizeof(result)));
    }
    catch (std::exception& e) {
        host->socket.close();
        return false;
    }
    if (result == 1) {
        awaiting_hosts_[host->get_name()] = host;
        return true;
    }
    return false;
}

std::vector<std::string> AwaitingHost::waiting_hosts() {
    // std::lock_guard<std::mutex> guard(mutex_);
    std::vector<std::string> results;
    for (auto& iter : awaiting_hosts_) {
        results.push_back(iter.first);
    }
    return results;
}
