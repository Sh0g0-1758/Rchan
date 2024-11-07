#include "RchanClient.cc"

void showWelcomeBanner()
{
  std::cout << Color::CYAN << Color::BOLD;
  std::cout << R"(
 /$$$$$$$   /$$$$$$  /$$   /$$  /$$$$$$  /$$   /$$
| $$__  $$ /$$__  $$| $$  | $$ /$$__  $$| $$$ | $$
| $$  \ $$| $$  \__/| $$  | $$| $$  \ $$| $$$$| $$
| $$$$$$$/| $$      | $$$$$$$$| $$$$$$$$| $$ $$ $$
| $$__  $$| $$      | $$__  $$| $$__  $$| $$  $$$$
| $$  \ $$| $$    $$| $$  | $$| $$  | $$| $$\  $$$
| $$  | $$|  $$$$$$/| $$  | $$| $$  | $$| $$ \  $$
|__/  |__/ \______/ |__/  |__/|__/  |__/|__/  \__/
    )" << Color::RESET
            << std::endl;

  std::cout << Color::YELLOW << "Welcome to Rchan - Your Secure Chat Platform!" << Color::RESET << "\n";
  std::cout << "═══════════════════════════════════════════════\n";
}

int main()
{
  try {
    showWelcomeBanner();
    RchanClient client;
    client.getUserName();

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
        if(std::find(client.getAvailableServers().begin(), client.getAvailableServers().end(), server_name) == client.getAvailableServers().end()) {
          std::cout << "Server does not exist\n";
          continue;
        }
        if (server_name == "Rchan") {
          client.EnterServer( client.getRchanIP(), 8080);
        } else {
          client.sendPassword( server_name );
        }
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