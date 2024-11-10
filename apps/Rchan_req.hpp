#pragma once

// common
#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <streambuf>
#include <thread>
#include <vector>
#include <mutex>
#include <fstream>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
#include <ncurses.h>
#endif

// gui.hh
#ifdef RCHAN_GUI
#include <ncurses.h>
#include <atomic>
#endif

namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string BOLD    = "\033[1m";
}

std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buffer;
}

std::pair<std::string, std::vector<json>> splitJSON(const std::string msg) {
    std::vector<json> messages;
    if(msg[0] != '{' or msg.size() <= 2)
        return {msg, messages};
    unsigned int start_brace = 0;
    unsigned int end_brace = 1;
    int brace_count = 1;
    while(end_brace < msg.size()) {
        if(msg[end_brace] == '{') {
            brace_count++;
        } else if(msg[end_brace] == '}') {
            brace_count--;
        }
        if(brace_count == 0) {
            messages.push_back(json::parse(msg.substr(start_brace, end_brace - start_brace + 1)));
            start_brace = end_brace + 1;
        }
        end_brace++;
    }
    if(start_brace < msg.size()) {
        return {msg.substr(start_brace), messages};
    } else {
        return {"", messages};
    }
}
