#pragma once

#include <string>
#include <string_view>

namespace base64 {

[[nodiscard]] std::string encode(std::string_view view);

};
