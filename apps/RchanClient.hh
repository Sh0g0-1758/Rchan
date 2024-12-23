#define RCHAN_CLIENT

#include "Rchan_req.hpp"

#include "local_server.cc"

class RchanClient
{
private:
  std::atomic<bool> running;
  std::string username;
  std::unique_ptr<RchanSocket> RsockPtr;
  std::string RchanServerIP;
  std::unique_ptr<std::thread> RchanClientPtr;
  std::vector<std::string> servers;
  std::mutex socketMutex;
  std::string fragment_store;
  std::unique_ptr<LocalServer> localServerPtr;
  std::string current_server;

  void listenForMessages( std::unique_ptr<RchanSocket>& sock );
  void set_current_server(std::string server) { current_server = server; }

public:
  RchanClient();
  ~RchanClient();

  RchanClient( const RchanClient& ) = delete;
  RchanClient& operator=( const RchanClient& ) = delete;

  void EnterServer( std::string server_name, std::string server_ip, int server_port );
  void HostServer();
  void unHostServer();
  void getUserName();
  void getChatHistory();
  void sendMessage( std::string message );
  void getServers();
  void sendPassword( std::string server_name );
  std::string getRchanIP() { return RchanServerIP; }
  std::vector<std::string> getAvailableServers() { return servers; }
  std::string get_current_server() { return current_server; }
};
