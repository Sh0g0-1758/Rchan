#define RCHAN_CLIENT
#include "Rchan_req.hpp"

#include "local_server.cc"
#include <fstream>
#include <ncurses.h>
#include <vector>
#include <sstream>


std::ofstream serverstream("window2.txt",std::ios::app);
std::ofstream errorstream("window1.txt",std::ios::app);
int max_y, max_x;
WINDOW* left_win,*right_win,*bottom_win;
const int BOTTOM_HEIGHT = 3;

std::string get_input(std::string message) {
    werase(bottom_win);
    box(bottom_win, 0, 0);
    mvwprintw(bottom_win, 1, 1, "%s", message.c_str());
    
    // Move cursor to input position
    wmove(bottom_win, 1, message.length() + 1);
    wrefresh(bottom_win);
    
    std::string input;
    
    while(true) {
        int ch = wgetch(bottom_win);
        
        if(ch == '\n') {
            break;
        }
        else if(ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            if(!input.empty()) {
                input.pop_back();
                wmove(bottom_win, 1, message.length() + 1);
                wclrtoeol(bottom_win);  // Clear from cursor to end of line
                box(bottom_win, 0, 0);  // Restore the box
                mvwprintw(bottom_win, 1, message.length() + 1, "%s", input.c_str());
            }
        }
        else if(ch != ERR && ch < 127 && ch >= 32) {
            input.push_back(ch);
            waddch(bottom_win, ch);
        }
        wrefresh(bottom_win);
    }
    
    return input;
}

class RchanClient {
private:
    std::atomic<bool> running{true};
    std::string username{};
    std::unique_ptr<RchanSocket> RsockPtr{};
    std::string RchanServerIP{};
    std::unique_ptr<std::thread> RchanClientPtr{};
    // server_name -> server_ip
    std::map<std::string, std::string> servers{};
    std::mutex socketMutex{};
    std::string fragment_store{};
    std::unique_ptr<LocalServer> localServerPtr{};
    
public:
    void listenForMessages(std::unique_ptr<RchanSocket>& sock) {
        
        static std::string buffer;

        while (running && !sock -> eof()) {
            sock -> read(buffer);
            if (!buffer.empty()) {
                // std::cout << "Received: " << buffer << std::endl;
                std::pair<std::string, std::vector<json>> split = splitJSON(buffer);
                std::vector<json> messages = split.second;
                // std::cout << messages.size() << std::endl;
                fragment_store += split.first;
                split = splitJSON(fragment_store);
                for(auto it : split.second) {
                    messages.push_back(it);
                }
                fragment_store = split.first;
                for(const json& message : messages) {
                    // std::cout << "Message: " << message.dump(4) << std::endl;
                    if(message["status"].get<std::string>() == "error") {
                        if(message["type"] == "username") {
                            serverstream << "Error: " << message["message"].get<std::string>() << std::endl;
                            getUserName();
                        } else {
                            serverstream << "Error: " << message["message"].get<std::string>() << std::endl;
                        }
                    } else if (message["type"].get<std::string>() == "chat") {
                        serverstream << message["message"].get<std::string>() << std::endl;
                    } else if (message["type"].get<std::string>() == "username") {
                        serverstream << message["message"].get<std::string>() << std::endl;
                    } else if (message["type"] == "available_servers") {
                        servers = message["servers"].get<std::map<std::string, std::string>>();
                        for(auto& server : servers) {
                            serverstream << server.first << " -> " << server.second << std::endl;
                        }
                    } else if (message["type"].get<std::string>() == "add_server") {
                        serverstream << "Added local server to Rchan!\n";
                        localServerPtr = std::make_unique<LocalServer>(message["server_port"].get<int>());
                        std::thread([this]() { localServerPtr -> Run(); }).detach();
                    } else if (message["type"].get<std::string>() == "remove_server") {
                        serverstream << "Removed local server from Rchan!\n";
                        localServerPtr.reset();
                    } else if (message["type"].get<std::string>() == "chat_history") {
                        serverstream << message["message"].get<std::string>() << std::endl;
                        serverstream << "Chat history loaded!\n";
                    }
                }
            }
            buffer.clear();
        }
    }

    RchanClient() {
        // RchanServerIP = "10.81.92.228";
        RchanServerIP = "10.81.38.165";
        {
            std::lock_guard<std::mutex> lock(socketMutex);
            RsockPtr = std::make_unique<RchanSocket>();
            RsockPtr->connect(Address(RchanServerIP, 8080));
        }
        errorstream << "Connected to Server" << std::endl;
        RchanClientPtr = std::make_unique<std::thread>([this]() { listenForMessages(std::ref(RsockPtr)); });
        RchanClientPtr->detach();
    }

    ~RchanClient() {
        running = false;
        if (RchanClientPtr && RchanClientPtr->joinable()) {
            RchanClientPtr->join();
        }
        RsockPtr -> wait_until_closed();
        RsockPtr.reset();
    }

    std::pair<std::string, int> parseIpPort(const std::string& address) {
        size_t colonPos = address.find(':');
        if (colonPos == std::string::npos) {
            throw std::invalid_argument("Invalid address format: missing ':'");
        }
        
        std::string ip = address.substr(0, colonPos);
        int port = std::stoi(address.substr(colonPos + 1));
        
        return {ip, port};
    }


    void EnterServer(std::string server_name) {
        if (servers.find(server_name) == servers.end()) {
            errorstream << "Error: Server " << server_name << " not found!" << std::endl;
            return;
        }

        // First stop the thread
        running = false;

        // Wait for thread to finish
        if (RchanClientPtr && RchanClientPtr->joinable()) {
            RchanClientPtr->join();
        }

        RsockPtr -> wait_until_closed();
        RsockPtr.reset();

        // Give some time for the socket cleanup
        // std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        try {
            {
                std::lock_guard<std::mutex> lock(socketMutex);
                RsockPtr = std::make_unique<RchanSocket>();
                std::pair<std::string, int> ipPort = parseIpPort(servers[server_name]);
                RsockPtr->connect(Address(ipPort.first, ipPort.second));
            }

            running = true;
            RchanClientPtr = std::make_unique<std::thread>([this]() { 
                listenForMessages(std::ref(RsockPtr)); 
            });
            RchanClientPtr->detach();

            getChatHistory();
        }
        catch (const std::exception& e) {
            errorstream << "Error connecting to server: " << e.what() << std::endl;
            running = false;
        }
    }

    void HostServer() {
        std::cout << "Enter server IP> ";
        std::string host_server_ip;
        std::getline(std::cin >> std::ws, host_server_ip);
        std::cout << "Enter server port> ";
        int host_server_port;
        std::cin >> host_server_port;
        std::cout << "Enter server name> ";
        std::string server_name;
        std::getline(std::cin >> std::ws, server_name);
        std::cout << "Enter root password> ";
        std::string root_password;
        std::getline(std::cin >> std::ws, root_password);
        std::cout << "Enter server password> ";
        std::string server_password;
        std::getline(std::cin >> std::ws, server_password);
        json message = {
            {"type", "add_server"},
            {"server_name", server_name},
            {"server_ip", host_server_ip},
            {"server_port", host_server_port},
            {"root_password", root_password},
            {"server_password", server_password}
        };
        {
            std::lock_guard<std::mutex> lock(socketMutex);
            RsockPtr -> write(message.dump());
        }
    }

    void unHostServer() {
        std::cout << "Enter server name> ";
        std::string unhost_server_name;
        std::getline(std::cin >> std::ws, unhost_server_name);
        std::cout << "Enter root password> ";
        std::string root_password;
        std::getline(std::cin >> std::ws, root_password);
        json message = {
            {"type", "remove_server"},
            {"server_name", unhost_server_name},
            {"root_password", root_password}
        };
        {
            std::lock_guard<std::mutex> lock(socketMutex);
            RsockPtr -> write(message.dump());
        }
    }



    void getUserName() {
        std::string username = get_input("Enter Username> ");
        
        
        json message = {
            {"type", "username"},
            {"username", username}
        };
        {
            std::lock_guard<std::mutex> lock(socketMutex);
            RsockPtr -> write(message.dump());
        }
    }

    void getChatHistory() {
        json message = {
            {"type", "chat_history"}
        };
        {
            std::lock_guard<std::mutex> lock(socketMutex);
            RsockPtr -> write(message.dump());
        }
    }

    void sendMessage(std::string message) {
        json messageJson = {
            {"type", "chat"},
            {"username", username},
            {"message", message}
        };
        {
            std::lock_guard<std::mutex> lock(socketMutex);
            RsockPtr -> write(messageJson.dump());
        }
    }

    void getServers() {
        json message = {
            {"type", "get_servers"}
        };
        {
            std::lock_guard<std::mutex> lock(socketMutex);
            RsockPtr -> write(message.dump());
        }
    }
};

std::atomic<bool> running{true};

void update_right_window() {
    std::string line;
    std::ifstream file;
    int current_y = 1;  // Start from line 1 to avoid box border

    while(running) {
        werase(right_win);
        box(right_win, 0, 0);
        
        // Open and read the file
        file.open("window2.txt");
        current_y = 1;  // Reset position
        
        while(std::getline(file, line)) {
            mvwprintw(right_win, current_y++, 1, "%s", line.c_str());
        }
        
        file.close();
        

        werase(left_win);
        box(left_win, 0, 0);
        
        // Open and read the file
        file.open("window1.txt");
        current_y = 1;  // Reset position
        
        while(std::getline(file, line)) {
            mvwprintw(left_win, current_y++, 1, "%s", line.c_str());
        }
        
        file.close();

        wrefresh(right_win);
        wrefresh(left_win);
        // Wait a bit before next update
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// void print_banner() {
//     printw("                ..                                        \n");
//     printw("                ==.                                       \n");
//     printw("                                                          \n");
//     printw("             .......        .           .=+-              \n");
//     printw("     -====-=--------:----====:          .*#=      ..:::--=\n");
//     printw("----=+++++++*++++++==+++===+++------------------====++*+++\n");
//     printw("=====-###*=%##=*###=+###-###+-========-----======++==+*+#=\n");
//     printw("\n");  // Space between dome and text
//     printw("                  RRRRR      CCCCC    H   H    AAAAA    N   N\n");
//     printw("                  R    R    C     C   H   H   A     A   NN  N\n");
//     printw("                  RRRRR     C         HHHHH   AAAAAAA   N N N\n");
//     printw("                  R   R     C     C   H   H   A     A   N  NN\n");
//     printw("                  R    R     CCCCC    H   H   A     A   N   N\n");
//     refresh();
//     std::this_thread::sleep_for(std::chrono::milliseconds(500));
// }

int main() {
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
    getmaxyx(stdscr, max_y, max_x);
    left_win = newwin(max_y-BOTTOM_HEIGHT, max_x/2 - 0.5, 0, 0);
    right_win = newwin(max_y-BOTTOM_HEIGHT, max_x/2 -0.5, 0, max_x/2+0/5);
    bottom_win = newwin(BOTTOM_HEIGHT, max_x, max_y - BOTTOM_HEIGHT, 0);
    // clear();
    // refresh();
    // wrefresh(left_win);
    //  wrefresh(right_win);
    //   wrefresh(bottom_win);
    // Enable scrolling for both windows
    scrollok(left_win, TRUE);
    scrollok(right_win, TRUE);
    scrollok(bottom_win,TRUE);
    
    // Enable keypad for both windows
    keypad(left_win, TRUE);
    keypad(right_win, TRUE);
    keypad(bottom_win, TRUE);

    std::thread update_thread(update_right_window);
    // std::thread error_thread(update_left_window);
    std::string input;
    
    try {

        RchanClient client;
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        client.getUserName();
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        // Add error handling for server entry
        while(true) {
            // input = get_input("Choose command> host server, enter server, send message, unhost server, get servers, exit> ");
            input = get_input("Choose command> ");

            if(input == "host server") {
                client.HostServer();
            } else if(input == "enter server") {
                input = get_input("Enter server name> ");
                client.EnterServer(input);
                // std::this_thread::sleep_for(std::chrono::seconds(1));
            } else if(input == "send message") {
                input = get_input("send message> ");
                client.sendMessage(input);
                // std::this_thread::sleep_for(std::chrono::seconds(1));
            } else if(input == "unhost server") {
                client.unHostServer();
            } else if(input == "get servers") {
                client.getServers();
            } else if(input == "exit") {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    running = false;
    if(update_thread.joinable()) update_thread.join();
    // if(error_thread.joinable()) error_thread.join();
    
    delwin(left_win);
    delwin(right_win);
    delwin(bottom_win);
    endwin();
    return 0;
}