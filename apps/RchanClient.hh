#define RCHAN_CLIENT

#include "Rchan_req.hpp"

#include "local_server.cc"

class RchanClient {
private:
    std::atomic<bool> running;
    std::string username;
    std::unique_ptr<RchanSocket> RsockPtr;
    std::string RchanServerIP;
    std::unique_ptr<std::thread> RchanClientPtr;
    std::map<std::string, std::string> servers;
    std::mutex socketMutex;
    std::string fragment_store;
    std::unique_ptr<LocalServer> localServerPtr;

    void listenForMessages(std::unique_ptr<RchanSocket>& sock);
    std::pair<std::string, int> parseIpPort(const std::string& address);

public:
    RchanClient();
    ~RchanClient();

    // Delete copy constructor and assignment operator due to unique_ptr members
    RchanClient(const RchanClient&) = delete;
    RchanClient& operator=(const RchanClient&) = delete;

    void EnterServer(std::string server_name);
    void HostServer();
    void unHostServer();
    void getUserName();
    void getChatHistory();
    void sendMessage(std::string message);
    void getServers();
};
