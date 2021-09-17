/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#pragma once

#include "fwd.hpp"

namespace util {

class prbs {
  static constexpr uint32_t prime = 79873;

public:
  void create_sequence(util::byte_span buf);

  bool check_sequence(util::byte_span buf);

  void reset();

private:
  bool sub_check_sequence(util::byte_span buf);

  uint32_t signature_ = 0x1;
};

} // namespace util
