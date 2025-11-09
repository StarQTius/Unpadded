#pragma once

#include <cstddef>

#include "record_element.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd {

template<std::size_t I, record_like Record>
  requires(I < record_size_v<Record>)
struct ith_record_element {
  using type = record_element_t<record_tag_v<I, Record>, Record>;
};

template<std::size_t I, record_like Record>
  requires(I < record_size_v<Record>)
using ith_record_element_t = typename ith_record_element<I, Record>::type;

} // namespace upd
