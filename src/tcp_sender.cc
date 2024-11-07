#include "tcp_sender.hh"
#include "tcp_config.hh"
#include <iostream>

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  uint64_t start = 0;
  uint64_t end = 0;
  if ( !outstanding_msg.empty() ) {
    start = outstanding_msg.front().seqno.unwrap( isn_, 0 );
    end = ( outstanding_msg.back().seqno + outstanding_msg.back().sequence_length() ).unwrap( isn_, 0 );
  }
  return end - start;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_ret;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  TCPSenderMessage msg;
  if ( seqno == isn_ ) {
    msg.SYN = true;
  }
  msg.seqno = seqno;
  uint64_t temp_window_size = window_size;
  if ( temp_window_size == 0 )
    temp_window_size = 1;
  uint64_t payload_size = min( temp_window_size - sequence_numbers_in_flight(), TCPConfig::MAX_PAYLOAD_SIZE );
  msg.payload = input_.reader().peek().substr( 0, payload_size );
  input_.reader().pop( payload_size );
  if ( input_.reader().is_finished()
       and ( (int64_t)temp_window_size - (int64_t)sequence_numbers_in_flight() - (int64_t)msg.sequence_length() )
             > 0
       and !FIN_SENT ) {
    msg.FIN = true;
    FIN_SENT = true;
  }
  if ( input_.has_error() ) {
    msg.RST = true;
  }
  if ( msg.sequence_length() != 0 ) {
    outstanding_msg.push( msg );
    seqno = seqno + msg.sequence_length();
    transmit( msg );
  }
  if ( temp_window_size - sequence_numbers_in_flight() > 0 and !input_.reader().peek().empty() )
    push( transmit );
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage msg;
  msg.seqno = seqno;
  if ( input_.has_error() ) {
    msg.RST = true;
  }
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  window_size = msg.window_size;
  if ( msg.RST )
    input_.reader().set_error();
  if ( !msg.ackno.has_value() or msg.ackno.value().unwrap( isn_, 0 ) > seqno.unwrap( isn_, 0 ) )
    return;
  while ( !outstanding_msg.empty()
          && ( outstanding_msg.front().seqno + outstanding_msg.front().sequence_length() ).unwrap( isn_, 0 )
               <= msg.ackno.value().unwrap( isn_, 0 ) ) {
    outstanding_msg.pop();
    consecutive_ret = 0;
    RTO = initial_RTO_ms_;
    timer = 0;
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  timer += ms_since_last_tick;
  if ( !outstanding_msg.empty() ) {
    if ( timer >= RTO ) {
      if ( window_size != 0 ) {
        consecutive_ret++;
        RTO = RTO * 2;
      }
      timer = 0;
      transmit( outstanding_msg.front() );
    }
  }
}
