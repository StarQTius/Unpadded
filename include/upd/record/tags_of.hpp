#pragma once

#include <tuple>

#include "../constexpr.hpp"
#include "../with_sequence.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd {

template<record_like Record>
struct tags_of {
  constexpr static auto value = UPD_WITH_SEQUENCE(Is, record_size_v<Record>) {
    return std::tuple{expr<record_tag_v<Is, Record>>...};
  };
};

template<record_like Record>
constexpr auto tags_of_v = tags_of<Record>::value;

} // namespace upd
