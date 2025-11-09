#pragma once

#include <concepts>
#include <cstddef>

#include "../upd.hpp"
#include "../variadic_concept.hpp"
#include "record_element.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd {

template<typename View, std::size_t I>
concept ith_element_viewer = requires(View view, record_tag<I, View> tag, record_element<tag.value, View> elem) {
  { get<tag.value>(UPD_FWD(view)) } -> std::same_as<typename decltype(elem)::type>;
};

template<typename Record>
concept record_view = record_like<Record> && UPD_ALL_OF_CONCEPT(ith_element_viewer, Record, record_size<Record>::value);

} // namespace upd
