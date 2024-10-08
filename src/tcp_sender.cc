#include "tcp_sender.hh"
#include "tcp_config.hh"
#include <iostream>

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  uint64_t start = 0;
  uint64_t end = 0;
  if(!outstanding_msg.empty()) {
    start = outstanding_msg.front().seqno.unwrap(isn_, 0);
    end = (outstanding_msg.back().seqno + outstanding_msg.back().sequence_length()).unwrap(isn_, 0);
  }
  return end - start;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_ret;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  if(to_send_seqno.unwrap(isn_, 0) > seqno.unwrap(isn_, 0)) return;
  TCPSenderMessage msg;
  if (seqno == isn_) {
    msg.SYN = true;
  }
  msg.seqno = seqno;
  // If window size is 0, we still send a payload of size 1
  uint64_t payload_size = max(min(window_size - sequence_numbers_in_flight(), MAX_PAYLOAD_SIZE), (size_t)1);
  msg.payload = input_.reader().peek().substr(0, payload_size);
  input_.reader().pop(payload_size);
  if (input_.reader().is_finished()) {
    msg.FIN = true;
  }
  if (input_.has_error()) {
    msg.RST = true;
  }
  if(msg.sequence_length() != 0) {
    outstanding_msg.push(msg);
    seqno = seqno + msg.sequence_length();
    to_send_seqno = to_send_seqno + msg.sequence_length();
    transmit(msg);
  } 
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage msg;
  msg.seqno = to_send_seqno;
  return msg;
}

void TCPSender::
receive( const TCPReceiverMessage& msg )
{
  seqno = msg.ackno.value();
  window_size = msg.window_size;
  while(!outstanding_msg.empty() && (outstanding_msg.front().seqno + outstanding_msg.front().sequence_length()).unwrap(isn_, 0) <= seqno.unwrap(isn_, 0)) {
    outstanding_msg.pop();
  }
  consecutive_ret = 0;
  RTO = initial_RTO_ms_;
  timer = 0;
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  timer += ms_since_last_tick;
  if(!outstanding_msg.empty()) {
    if(timer >= RTO) {
      consecutive_ret++;
      transmit(outstanding_msg.front());
      RTO = RTO * 2;
      timer = 0;
    }
  }
}
