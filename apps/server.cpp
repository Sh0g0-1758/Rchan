#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <csignal>
#include <fstream>
#include <ctime>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

std::vector<int> clients;
std::mutex clientsMutex;

void writeMessageToFile(const std::string &message) {
    std::ofstream outFile("chat_history.txt", std::ios::app);
    if (outFile.is_open()) {
        outFile << message << std::endl;
        outFile.close();
    }
}

void sendChatHistory(int clientSocket) {
    std::ifstream inFile("chat_history.txt");
    std::string line;

    std::string chatHistory;
    while (std::getline(inFile, line)) {
        chatHistory += line + "\n";
    }
    inFile.close();
    send(clientSocket, chatHistory.c_str(), chatHistory.length(), 0);
}

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE] = {0};
    sendChatHistory(clientSocket);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE); 
        int bytesRead = read(clientSocket, buffer, BUFFER_SIZE);

        if (bytesRead <= 0) {
            std::cout << "Client disconnected.\n";
            break;
        }

        std::string message(buffer, bytesRead);
        writeMessageToFile(message);

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            for (int client : clients) {
                if (client != clientSocket) {
                    send(client, buffer, bytesRead, 0);
                }
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }

    close(clientSocket);
}

int main() {
    int server_fd, new_socket;
    sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Chat server listening on port " << PORT << "...\n";

    while (true) {
        if ((new_socket = accept(server_fd, (sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::cout << new_socket << " connected\n";
            clients.push_back(new_socket);
        }

        std::thread(handleClient, new_socket).detach();
    }
}
