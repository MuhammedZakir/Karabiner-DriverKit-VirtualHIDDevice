#pragma once

// (C) Copyright Takayama Fumihiko 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#include <cstdint>

namespace pqrs {
namespace karabiner {
namespace driverkit {
namespace virtual_hid_device_driver {
namespace hid_report {

class __attribute__((packed)) do_not_disturb_input final {
public:
  do_not_disturb_input(void) : report_id_(7), do_not_disturb(0) {}
  bool operator==(const do_not_disturb_input& other) const { return (memcmp(this, &other, sizeof(*this)) == 0); }
  bool operator!=(const do_not_disturb_input& other) const { return !(*this == other); }

private:
  uint8_t report_id_ __attribute__((unused));

public:
  uint8_t do_not_disturb;
};

} // namespace hid_report
} // namespace virtual_hid_device_driver
} // namespace driverkit
} // namespace karabiner
} // namespace pqrs
