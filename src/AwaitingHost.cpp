#include <memory>
#include "AwaitingHost.h"
#include "Bridge.h"

void AwaitingHost::remove(const std::string& name) {
    std::lock_guard<std::mutex> guard(mutex_);
    awaiting_hosts_.erase(name);
}


std::shared_ptr<Bridge> AwaitingHost::match_host(const std::string& name, std::shared_ptr<ClientConnection> client) {
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = awaiting_hosts_.find(name);
    if (iter == awaiting_hosts_.end()) return false;
    auto bridge = std::make_shared<Bridge>(iter->second, client);
    awaiting_hosts_.erase(iter);
    return bridge;
}

bool AwaitingHost::enlist(std::shared_ptr<HostConnection> host) {
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = awaiting_hosts_.find(host->getName());
    if (iter != awaiting_hosts_.end()) return false;
    awaiting_hosts_[host->getName()] = host;
}