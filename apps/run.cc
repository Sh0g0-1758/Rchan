#include<iostream>
using namespace std;

#include "client.cc"
#include "server.cc"

int main() {
    RchanClient client;

    std::cout << "Enter Username> ";
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