/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#include "util/prbs.hpp"

namespace {

template <class T>
bool is_odd(T t) {
  return (t & 0x01);
}

} // namespace

namespace util {

// void prbs::create_sequence(util::byte_span buf) {
//   auto is_odd = [](auto val) { return (val & 0x01) == 0x01; };
//   auto step = [&]() {
//     if (is_odd(signature_))
//       signature_ ^= prime;
//     signature_ >>= 1;
//   };
//   auto end = is_odd(buf.size()) ? std::prev(buf.end()) : buf.end();
//   auto it = buf.begin();
//   while (it != end) {
//     step();
//     *it++ = std::byte(signature_ & 0xff);
//     *it++ = std::byte(signature_ >> 8);
//   }
//   if (is_odd(buf.size())) {
//     step();
//     buf.back() = std::byte(signature_ & 0xff);
//   }
// }

// bool prbs::sub_check_sequence(util::byte_span buf) {
//   auto is_odd = [](auto val) { return (val & 0x01) == 0x01; };
//   auto end = is_odd(buf.size()) ? std::prev(buf.end()) : buf.end();
//   auto it = buf.begin();
//   auto magic = static_cast<uint32_t>(*it++ | (*it++ << 8));
//   auto step = [&]() {
//     if (is_odd(magic))
//       magic ^= prime;
//     magic >>= 1;
//   };
//   while (it != end) {
//     step();
//     if (magic != static_cast<uint32_t>(*it++ | (*it++ << 8)))
//       return false;
//   }
//   if (is_odd(buf.size())) {
//     step();
//     if ((magic >> 8) != static_cast<uint32_t>(buf.back()))
//       return false;
//   }
//   return true;
// }

void prbs::create_sequence(util::byte_span buf) {
  const uint16_t max = buf.size() & ~0x01;
  uint16_t i = 0;
  while (i < max) {
    if (signature_ & 0x01)
      signature_ ^= prime;
    signature_ >>= 1;
    buf[i++] = std::byte(signature_ & 0xff);
    buf[i++] = std::byte(signature_ >> 8);
  }
  if (buf.size() & 0x01) {
    if (signature_ & 0x01)
      signature_ ^= prime;
    signature_ >>= 1;
    buf.back() = std::byte(signature_ & 0xff);
  }
}

bool prbs::check_sequence(util::byte_span buf) {
  if (buf.size() >= 4) {
    if (sub_check_sequence(buf))
      return true;
    else
      return sub_check_sequence(buf.subspan(1));
  }
  return false;
}

void prbs::reset() {
  signature_ = 0x1;
}

bool prbs::sub_check_sequence(util::byte_span buf) {
  const uint16_t max = buf.size() & ~0x01;
  auto magic = static_cast<uint32_t>(buf[0] | (buf[1] << 8));
  size_t i = 2;
  while (i < max) {
    if (magic & 0x01)
      magic ^= prime;
    magic >>= 1;
    if (magic != static_cast<uint32_t>(buf[i] | (buf[i + 1] << 8)))
      return false;
    i += 2;
  }
  if (buf.size() & 0x01) {
    if (magic & 0x01)
      magic ^= prime;
    magic >>= 1;
    if ((magic >> 8) != static_cast<uint32_t>(buf.back()))
      return false;
  }
  return true;
}

} // namespace util
