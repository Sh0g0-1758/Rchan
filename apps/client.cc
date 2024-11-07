#define RCHAN_CLIENT

#include "Rchan_req.hpp"

#include "local_server.cc"

class RchanClient
{
private:
  std::atomic<bool> running { true };
  std::string username {};
  std::unique_ptr<RchanSocket> RsockPtr {};
  std::string RchanServerIP {};
  std::unique_ptr<std::thread> RchanClientPtr {};
  // server_name -> server_ip
  std::map<std::string, std::string> servers {};
  std::mutex socketMutex {};
  std::string fragment_store {};
  std::unique_ptr<LocalServer> localServerPtr {};

public:
  void listenForMessages( std::unique_ptr<RchanSocket>& sock )
  {
    static std::string buffer;

    while ( running && !sock->eof() ) {
      sock->read( buffer );
      if ( !buffer.empty() ) {
        std::pair<std::string, std::vector<json>> split = splitJSON( buffer );
        std::vector<json> messages = split.second;
        fragment_store += split.first;
        split = splitJSON( fragment_store );
        for ( auto it : split.second ) {
          messages.push_back( it );
        }
        fragment_store = split.first;
        for ( const json& message : messages ) {
          if ( message["status"].get<std::string>() == "error" ) {
            if ( message["type"] == "username" ) {
              std::cout << "Error: " << message["message"].get<std::string>() << std::endl;
              getUserName();
            } else {
              std::cout << "Error: " << message["message"].get<std::string>() << std::endl;
            }
          } else if ( message["type"].get<std::string>() == "chat" ) {
            std::cout << message["message"].get<std::string>() << std::endl;
          } else if ( message["type"].get<std::string>() == "username" ) {
            std::cout << message["message"].get<std::string>() << std::endl;
          } else if ( message["type"] == "available_servers" ) {
            servers = message["servers"].get<std::map<std::string, std::string>>();
            for ( auto& server : servers ) {
              std::cout << server.first << " -> " << server.second << std::endl;
            }
          } else if ( message["type"].get<std::string>() == "add_server" ) {
            std::cout << "Added local server to Rchan!\n";
            localServerPtr = std::make_unique<LocalServer>( message["server_port"].get<int>() );
            std::thread( [this]() { localServerPtr->Run(); } ).detach();
          } else if ( message["type"].get<std::string>() == "remove_server" ) {
            std::cout << "Removed local server from Rchan!\n";
            localServerPtr.reset();
          } else if ( message["type"].get<std::string>() == "chat_history" ) {
            std::cout << message["message"].get<std::string>() << std::endl;
            std::cout << "Chat history loaded!\n";
          }
        }
      }
      buffer.clear();
    }
  }

  RchanClient()
  {
    RchanServerIP = "10.81.92.228";
    {
      std::lock_guard<std::mutex> lock( socketMutex );
      RsockPtr = std::make_unique<RchanSocket>();
      RsockPtr->connect( Address( RchanServerIP, 8080 ) );
    }
    std::cout << "Connected to Server" << std::endl;
    RchanClientPtr = std::make_unique<std::thread>( [this]() { listenForMessages( std::ref( RsockPtr ) ); } );
    RchanClientPtr->detach();
  }

  ~RchanClient()
  {
    running = false;
    if ( RchanClientPtr && RchanClientPtr->joinable() ) {
      RchanClientPtr->join();
    }
    RsockPtr->wait_until_closed();
    RsockPtr.reset();
  }

  std::pair<std::string, int> parseIpPort( const std::string& address )
  {
    size_t colonPos = address.find( ':' );
    if ( colonPos == std::string::npos ) {
      throw std::invalid_argument( "Invalid address format: missing ':'" );
    }

    std::string ip = address.substr( 0, colonPos );
    int port = std::stoi( address.substr( colonPos + 1 ) );

    return { ip, port };
  }

  void EnterServer( std::string server_name )
  {
    if ( servers.find( server_name ) == servers.end() ) {
      std::cout << "Error: Server " << server_name << " not found!" << std::endl;
      return;
    }

    // First stop the thread
    running = false;

    // Wait for thread to finish
    if ( RchanClientPtr && RchanClientPtr->joinable() ) {
      RchanClientPtr->join();
    }

    RsockPtr->wait_until_closed();
    RsockPtr.reset();

    // Give some time for the socket cleanup
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    try {
      {
        std::lock_guard<std::mutex> lock( socketMutex );
        RsockPtr = std::make_unique<RchanSocket>();
        std::pair<std::string, int> ipPort = parseIpPort( servers[server_name] );
        RsockPtr->connect( Address( ipPort.first, ipPort.second ) );
      }

      running = true;
      RchanClientPtr = std::make_unique<std::thread>( [this]() { listenForMessages( std::ref( RsockPtr ) ); } );
      RchanClientPtr->detach();

      getChatHistory();
    } catch ( const std::exception& e ) {
      std::cout << "Error connecting to server: " << e.what() << std::endl;
      running = false;
    }
  }

  void HostServer()
  {
    std::cout << "Enter server IP> ";
    std::string host_server_ip;
    std::getline( std::cin >> std::ws, host_server_ip );
    std::cout << "Enter server port> ";
    int host_server_port;
    std::cin >> host_server_port;
    std::cout << "Enter server name> ";
    std::string server_name;
    std::getline( std::cin >> std::ws, server_name );
    std::cout << "Enter root password> ";
    std::string root_password;
    std::getline( std::cin >> std::ws, root_password );
    std::cout << "Enter server password> ";
    std::string server_password;
    std::getline( std::cin >> std::ws, server_password );
    json message = { { "type", "add_server" },
                     { "server_name", server_name },
                     { "server_ip", host_server_ip },
                     { "server_port", host_server_port },
                     { "root_password", root_password },
                     { "server_password", server_password } };
    {
      std::lock_guard<std::mutex> lock( socketMutex );
      RsockPtr->write( message.dump() );
    }
  }

  void unHostServer()
  {
    std::cout << "Enter server name> ";
    std::string unhost_server_name;
    std::getline( std::cin >> std::ws, unhost_server_name );
    std::cout << "Enter root password> ";
    std::string root_password;
    std::getline( std::cin >> std::ws, root_password );
    json message
      = { { "type", "remove_server" }, { "server_name", unhost_server_name }, { "root_password", root_password } };
    {
      std::lock_guard<std::mutex> lock( socketMutex );
      RsockPtr->write( message.dump() );
    }
  }

  void getUserName()
  {
    std::cout << "Enter username> ";
    std::getline( std::cin >> std::ws, username );
    json message = { { "type", "username" }, { "username", username } };
    {
      std::lock_guard<std::mutex> lock( socketMutex );
      RsockPtr->write( message.dump() );
    }
  }

  void getChatHistory()
  {
    json message = { { "type", "chat_history" } };
    {
      std::lock_guard<std::mutex> lock( socketMutex );
      RsockPtr->write( message.dump() );
    }
  }

  void sendMessage( std::string message )
  {
    json messageJson = { { "type", "chat" }, { "username", username }, { "message", message } };
    {
      std::lock_guard<std::mutex> lock( socketMutex );
      RsockPtr->write( messageJson.dump() );
    }
  }

  void getServers()
  {
    json message = { { "type", "get_servers" } };
    {
      std::lock_guard<std::mutex> lock( socketMutex );
      RsockPtr->write( message.dump() );
    }
  }
};

int main()
{
  try {
    RchanClient client;
    client.getUserName();
    // std::this_thread::sleep_for(std::chrono::seconds(1));

    // Add error handling for server entry
    while ( true ) {
      std::cout << "Choose command> host server, enter server, send message, unhost server, get servers, exit"
                << std::endl;
      std::string command;
      std::getline( std::cin >> std::ws, command );
      if ( command == "host server" ) {
        client.HostServer();
      } else if ( command == "enter server" ) {
        std::cout << "Enter server name> ";
        std::string server_name;
        std::getline( std::cin >> std::ws, server_name );
        client.EnterServer( server_name );
      } else if ( command == "send message" ) {
        std::cout << "Enter message> ";
        std::string message;
        std::getline( std::cin >> std::ws, message );
        client.sendMessage( message );
      } else if ( command == "unhost server" ) {
        client.unHostServer();
      } else if ( command == "get servers" ) {
        client.getServers();
      } else if ( command == "exit" ) {
        break;
      }
    }
  } catch ( const std::exception& e ) {
    std::cout << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}