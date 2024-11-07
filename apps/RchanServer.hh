#define RCHAN_SERVER

#include "Rchan_req.hpp"

struct server_info
{
  std::string server_name;
  std::string server_ip;
  int server_port;
  std::string root_password;
  std::string server_password;
};

class RchanServer
{
private:
  static const int PORT = 8080;
  static const int BUFFER_SIZE = 4096;

  std::vector<int> clients;
  std::map<std::string, std::string> servers;
  std::map<std::string, server_info> localRChanServers;
  std::mutex clientsMutex;
  std::vector<std::string> usernames;

  int server_fd;
  int new_socket;
  sockaddr_in address;
  int addrlen;
  int opt;

  std::string hashPSWD( const std::string& input );
  void handleClient( int clientSocket );
  void sendMessage( const nlohmann::json& message, int clientSocket );
  void sendChatHistory( int clientSocket );
  void sendAvailableServers( int clientSocket );
  void writeMessageToFile( const std::string& message );

public:
  RchanServer();
  ~RchanServer();

  RchanServer( const RchanServer& ) = delete;
  RchanServer& operator=( const RchanServer& ) = delete;

  void Run();
};