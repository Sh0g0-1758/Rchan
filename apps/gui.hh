#define RCHAN_GUI

#include "Rchan_req.hpp"

namespace RchanGUI {
    std::ofstream serverstream("window2.txt",std::ios::app);
    std::ofstream errorstream("window1.txt",std::ios::app);
    int max_y, max_x;
    WINDOW* left_win,*right_win,*bottom_win;
    const int BOTTOM_HEIGHT = 3;
    std::atomic<bool> GUI_running{true};

    std::string get_input(std::string message);
    void update_right_window();
}
