#include "Rchan_req.hpp"

int main() {
    std::vector<json> hello = splitJSON("{\"servers\":{\"Rchan\":\"10.81.92.228\"},\"status\":\"success\",\"type\":\"available_servers\"}");
    for(const json& message : hello) {
        std::cout << message.dump(4) << std::endl;
    }
}
