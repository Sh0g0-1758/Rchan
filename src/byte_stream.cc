#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {
  bytes.reserve(capacity); // reserve some space in the string
}

bool Writer::is_closed() const
{
  return is_closed_;
}

void Writer::push( string data )
{
  size_t space_left = capacity_ - currlen;
  size_t to_append = std::min(space_left, data.size());

  bytes.append(data, 0, to_append); // this avoids creation of another string
  pushcnt += to_append;
  currlen += to_append;
}

void Writer::close()
{
  is_closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - currlen;
}

uint64_t Writer::bytes_pushed() const
{
  return pushcnt;
}

bool Reader::is_finished() const
{
  return is_closed_ and currlen == 0;
}

uint64_t Reader::bytes_popped() const
{
  return popcnt;
}

string_view Reader::peek() const
{
  return std::string_view(bytes).substr(0); // doing this instead of returning bytes directly
}

void Reader::pop( uint64_t len )
{
  if (len >= currlen) {
    popcnt += currlen;
    currlen = 0;
    bytes = "";
  } else {
    popcnt += len;
    bytes.erase(0, len); // erase also takes care of the case when len > bytes.size() and it does not create another string
    currlen -= len;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return currlen;
}
