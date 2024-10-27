#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <fstream>
#include <ctime>
#include "socket.hh"
#include <cstdlib>
#include <memory>
#include <span>
#include <string>
#include <atomic>
#include "tcp_minnow_socket.hh"

std::atomic<bool> running{true};
std::string username;

std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buffer;
}

void listenForMessages(CS144TCPSocket& sock) {
    std::string buffer;

    while (running && !sock.eof()) {
        sock.read(buffer);
        if (!buffer.empty()) {
            std::cout << "\r" << buffer << "\n" << "[" << getCurrentTime() << "] " << username << "> " << std::flush;
        }
        buffer.clear();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    CS144TCPSocket sock;
    
    int enable = 1;
    if (setsockopt(sock.fd_num(), SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR\n";
        return 1;
    }
    
    if (setsockopt(sock.fd_num(), SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        std::cerr << "Failed to set SO_REUSEPORT\n";
        return 1;
    }

    std::string serverIP;
    std::cout << "Enter server IP address: ";
    std::cin >> serverIP;
    if (serverIP == "default") {
        serverIP = "10.81.92.228";
    }

    sock.connect(Address(serverIP, 8080));
    std::cout << "Connected to the server!\n";

    std::cout << "Type your username and press Enter: ";
    std::getline(std::cin >> std::ws, username);

    std::cout << "Hey " << username << "! Welcome to Rchan. Type 'exit' to quit\n";
    std::cout << "⏳ Loading chat history ⏳\n";

    std::thread listener([&sock]() { listenForMessages(std::ref(sock)); });
    listener.detach();

    while (running) {
        std::cout << "[" << getCurrentTime() << "] " << username << "> " << std::flush;
        std::string message;
        std::getline(std::cin, message);

        if (message == "exit") {
            running = false;
            break;
        }

        std::string timestampedMessage = "[" + getCurrentTime() + "] " + username + "> " + message + "\n";
        sock.write(timestampedMessage);
    }
    
    sock.close();
    return 0;
}
