#pragma once

// common
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <fstream>
#include <ctime>

// server.cc
#ifdef RCHAN_SERVER
#include <netinet/in.h>
#include <sys/socket.h>
#include <openssl/evp.h>
#endif

// client.cc
#ifdef RCHAN_CLIENT
#include "socket.hh"
#include "tcp_minnow_socket.hh"
#endif

#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buffer;
}
