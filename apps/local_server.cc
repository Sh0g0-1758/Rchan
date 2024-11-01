#undef  RCHAN_CLIENT
#define RCHAN_SERVER

#include "Rchan_req.hpp"

class LocalServer {
private:
    static const int PORT = 8080;
    static const int BUFFER_SIZE = 4096;

    std::vector<int> clients{};
    std::mutex clientsMutex{};
    int server_fd{}, new_socket{};
    sockaddr_in address{};
    int addrlen = sizeof(address);
    int opt = 1;
public:
    LocalServer() {
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

        std::cout << "Server listening on port " << PORT << "...\n";
    }

    ~LocalServer() {
        for(int client : clients) {
            close(client);
        }
    }

    void Run() {
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
            std::thread([this]() {
                this->handleClient(new_socket);
            }).detach();
        }
    }

    void writeMessageToFile(const std::string &message) {
        std::ofstream outFile("chat_history.txt", std::ios::app);
        if (outFile.is_open()) {
            outFile << message << std::endl;
            outFile.close();
        }
    }

    void sendMessage(const json& message, int clientSocket) {
        static std::string buffer;
        buffer.clear(); // Just resets length, keeps capacity
        buffer = message.dump();
        try {
            send(clientSocket, buffer.c_str(), buffer.length(), 0);
        } catch (const std::exception& e) {
            std::cout << "Error sending message: " << e.what() << std::endl;
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
        json message = {
            {"status", "success"},
            {"type", "chat_history"},
            {"message", chatHistory}
        };
        sendMessage(message, clientSocket);
    }

    void handleClient(int clientSocket) {
        char buffer[BUFFER_SIZE]{};
        std::string fragment_store{};
        std::cout << "Handling Client\n";

        while (true) {
            memset(buffer, 0, BUFFER_SIZE); 
            int bytesRead = read(clientSocket, buffer, BUFFER_SIZE);

            if (bytesRead <= 0) {
                std::cout << "Client disconnected.\n";
                break;
            }

            std::string REVCDmessage(buffer, bytesRead);
            std::cout << "Received: " << REVCDmessage << std::endl;
            std::pair<std::string, std::vector<json>> split = splitJSON(REVCDmessage);
            std::vector<json> messagesJSON = split.second;
            std::cout << messagesJSON.size() << std::endl;
            fragment_store += split.first;
            split = splitJSON(fragment_store);
            for(auto it : split.second) {
                messagesJSON.push_back(it);
            }
            fragment_store = split.first;
            for(const json& messageJSON : messagesJSON) {
                std::cout << messageJSON.dump(4) << std::endl;
                if (messageJSON["type"].get<std::string>() == "chat_history") {
                    sendChatHistory(clientSocket);
                } else if(messageJSON["type"].get<std::string>() == "chat") {
                    std::string username = messageJSON["username"].get<std::string>();
                    std::string msg = messageJSON["message"].get<std::string>();
                    std::string timestampedMessage = "[" + getCurrentTime() + "] " + username + "> " + msg + "\n";
                    writeMessageToFile(timestampedMessage);
                    json message = {
                        {"status", "success"},
                        {"type", "chat"},
                        {"message", timestampedMessage}
                    };
                    {
                        std::lock_guard<std::mutex> lock(clientsMutex);
                        for (int client : clients) {
                            sendMessage(message, client);
                        }
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
};
