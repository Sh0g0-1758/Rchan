#include "reassembler.hh"
#include <map>
#include <iostream>

void Reassembler::insert( uint64_t first_index, std::string data, bool is_last_substring )
{
  if (first_index >= curr_index + writer().available_capacity()) {
    return;
  }

  if (first_index < curr_index) {
    if (first_index + data.size() <= curr_index) {
      return;
    }
    data = data.substr(curr_index - first_index);
    first_index = curr_index;
  }

  if (first_index > curr_index) {
    uint64_t possible_length = curr_index + writer().available_capacity() - first_index;
    if (possible_length > data.size()) {
      possible_length = data.size();
    }
    if (pending_data.find(first_index) != pending_data.end()) {
      if (pending_data[first_index].size() < possible_length) {
        pending_data[first_index] = data.substr(0, possible_length);
      }
    } else {
      pending_data[first_index] = data.substr(0, possible_length);
    }
  } else if (first_index == curr_index) {
    uint64_t previous_push = writer().bytes_pushed();
    output_.writer().push(data);
    uint64_t pushed = writer().bytes_pushed() - previous_push;
    curr_index += pushed;
    for (auto it = pending_data.begin(); it != pending_data.end();) {
        if (it->first == curr_index) {
            previous_push = writer().bytes_pushed();
            output_.writer().push(it->second);
            pushed = writer().bytes_pushed() - previous_push;
            curr_index += pushed;
            it = pending_data.erase(it);
        } 
        else if (it->first < curr_index) {
            if (it->first + it->second.size() - 1 >= curr_index) {
                previous_push = writer().bytes_pushed();
                output_.writer().push(it->second.substr(curr_index - it->first));
                pushed = writer().bytes_pushed() - previous_push;
                curr_index += pushed;
                it = pending_data.erase(it);
            } 
            else {
                it = pending_data.erase(it);
            }
        } 
        else {
            ++it;
        }
    }
  }

  std::cout << "before segfault" << std::endl;
  if (is_last_substring) {
    last_index = first_index + data.size();
  }
  std::cout << "after segfault" << std::endl;

  if (last_index == curr_index) output_.writer().close();
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  std::map<uint64_t, bool> count;
  for(auto it : pending_data) {
    for(uint64_t i = 0; i < it.second.size(); i++) {
      count[it.first + i] = true;
    }
  }
  return count.size();
}
