#include "gui.hh"

std::string RchanGUI::get_input(std::string message) {
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

void RchanGUI::update_right_window() {
    std::string line;
    std::ifstream file;
    int current_y = 1;  // Start from line 1 to avoid box border

    while(GUI_running) {
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