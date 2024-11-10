#include "RchanClient.cc"
#include "gui.cc"

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
    std::ofstream clear_server("window2.txt", std::ios::trunc);
    std::ofstream clear_error("window1.txt", std::ios::trunc);
    clear_server.close();
    clear_error.close();

    initscr();
    start_color();
    // print_banner();
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    cbreak();
    noecho();
    keypad(stdscr, TRUE);  // Enable keyboard mapping

    // Enable scrolling
    scrollok(stdscr, TRUE);

    // Create two windows - left and right half
    getmaxyx(stdscr, RchanGUI::max_y, RchanGUI::max_x);
    RchanGUI::left_win = newwin(RchanGUI::max_y-RchanGUI::BOTTOM_HEIGHT, RchanGUI::max_x/2 - 0.5, 0, 0);
    RchanGUI::right_win = newwin(RchanGUI::max_y-RchanGUI::BOTTOM_HEIGHT, RchanGUI::max_x/2 -0.5, 0, RchanGUI::max_x/2+0/5);
    RchanGUI::bottom_win = newwin(RchanGUI::BOTTOM_HEIGHT, RchanGUI::max_x, RchanGUI::max_y - RchanGUI::BOTTOM_HEIGHT, 0);
    // clear();
    // refresh();
    // wrefresh(left_win);
    //  wrefresh(right_win);
    //   wrefresh(bottom_win);
    // Enable scrolling for both windows
    scrollok(RchanGUI::left_win, TRUE);
    scrollok(RchanGUI::right_win, TRUE);
    scrollok(RchanGUI::bottom_win,TRUE);
    
    // Enable keypad for both windows
    keypad(RchanGUI::left_win, TRUE);
    keypad(RchanGUI::right_win, TRUE);
    keypad(RchanGUI::bottom_win, TRUE);

    std::thread update_thread(RchanGUI::update_right_window);
    // std::thread error_thread(update_left_window);
    std::string command;
  try {
    showWelcomeBanner();
    RchanClient client;
    client.getUserName();

    while ( true ) {
      command = RchanGUI::get_input( "Choose command> host server, enter server, send message, unhost server, get servers, exit\n" );
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
          client.EnterServer( "Rchan", client.getRchanIP(), 8080);
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