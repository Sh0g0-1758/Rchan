#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if (message.SYN) {
    zero_point = message.seqno;
    ISN = true;
    ackno = zero_point + 1;
  }

  if (message.RST) reassembler_.reader().set_error();

  uint64_t stream_index = message.seqno.unwrap(zero_point, 0) - 1;
  if (message.SYN) stream_index++;

  uint32_t prior = reassembler_.writer().bytes_pushed();

  if (message.FIN) reassembler_.insert(stream_index, message.payload, true);
  else reassembler_.insert(stream_index, message.payload, false);

  ackno = ackno + (static_cast<uint32_t>(reassembler_.writer().bytes_pushed()) - prior);
  if (reassembler_.writer().is_closed()) ackno = ackno + 1;
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage message;
  if (ISN) message.ackno = ackno;
  uint64_t ws = reassembler_.writer().available_capacity();
  if (ws > UINT16_MAX) ws = UINT16_MAX;
  message.window_size = ws;
  message.RST = reassembler_.reader().has_error();
  return message;
}
