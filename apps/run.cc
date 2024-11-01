#include "Rchan_req.hpp"

int main() {
    std::vector<json> hello = splitJSON("{\"type\":\"chat_history\"}{\"message\":\"Hello World!\",\"type\":\"chat\",\"username\":\"shogo\"}");
    for(const json& message : hello) {
        std::cout << message.dump(4) << std::endl;
    }
}
