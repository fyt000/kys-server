#include <memory>
#include <iostream>
#include <asio.hpp>
#include "AwaitingHost.h"
#include "Bridge.h"

void AwaitingHost::remove(const std::string& name) {
    std::lock_guard<std::mutex> guard(mutex_);
    awaiting_hosts_.erase(name);
}

std::shared_ptr<Bridge> AwaitingHost::match_host(const std::string& name, std::shared_ptr<ClientConnection> client) {
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = awaiting_hosts_.find(name);
    if (iter == awaiting_hosts_.end()) return nullptr;
    auto bridge = std::make_shared<Bridge>(iter->second, client);
    awaiting_hosts_.erase(iter);
    return bridge;
}

bool AwaitingHost::enlist(std::shared_ptr<HostConnection> host) {
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = awaiting_hosts_.find(host->get_name());
    int result = 1;
    if (iter != awaiting_hosts_.end()) result = 0;
   
    try {
        // ok is only 0 or 1, timer check is 2
        boost::asio::write(host->socket, boost::asio::buffer(&result, sizeof(result)));
    }
    catch (std::exception& e) {
        std::cerr << "host err2 " << e.what() << std::endl;
        host->socket.close();
        // I don't have to close it, but I can, this is not thread safe
        return false;
    }
    if (result == 1) {
        awaiting_hosts_[host->get_name()] = host;
        return true;
    }
    return false;
}

std::vector<std::string> AwaitingHost::waiting_hosts() {
    std::lock_guard<std::mutex> guard(mutex_);
    std::vector<std::string> results;
    for (auto& iter : awaiting_hosts_) {
        results.push_back(iter.first);
    }
    return results;
}

void AwaitingHost::host_cleanup() {
    std::lock_guard<std::mutex> guard(mutex_);
    std::vector<std::string> hostnames;
    std::vector<std::future<std::size_t>> futures;
    int one = 1;
    for (auto it = awaiting_hosts_.begin(); it != awaiting_hosts_.end();) {
        try {
            it->second->socket.send(boost::asio::buffer(&one, sizeof(one)));
        } catch (std::exception & e) {
            it = awaiting_hosts_.erase(it);
            continue;
        }
    }
    /*
    for (auto& it : awaiting_hosts_) {
        futures.emplace_back(boost::asio::async_write(it.second->socket, boost::asio::buffer(&one, sizeof(one)), boost::asio::use_future));
        hostnames.push_back(it.first);
    }
    for (int i = 0; i < futures.size(); i++) {
        std::cerr << "checking " << hostnames[i] << std::endl;
        try {
            // need a worker to carry this out.. I guess
            futures[i].get();
        } catch (std::exception & e) {
            awaiting_hosts_.erase(hostnames[i]);
        }
    }*/
}