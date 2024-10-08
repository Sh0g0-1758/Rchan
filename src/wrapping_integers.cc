#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32((n + zero_point.raw_value_) % (1UL << 32));
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t divisor = 1UL << 32;
  uint64_t abs_seqno = raw_value_ >= zero_point.raw_value_ 
                     ? raw_value_ - zero_point.raw_value_ 
                     : divisor + raw_value_ - zero_point.raw_value_;

  if (checkpoint <= abs_seqno) return abs_seqno;
  uint64_t diff = checkpoint - abs_seqno;
  uint64_t quotient = diff / divisor;
  uint64_t remainder = diff % divisor;
  uint64_t k = quotient + (remainder >= (divisor / 2) ? 1 : 0);

  return abs_seqno + k * divisor;
}
