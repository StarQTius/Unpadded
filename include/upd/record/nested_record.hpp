#pragma once

#include <cstddef>

#include "../variadic_concept.hpp"
#include "record_element.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd {

template<typename Record, std::size_t I>
concept ith_record_element_record_like = record_like<record_element_t<record_tag_v<I, Record>, Record>>;

template<typename Record>
concept nested_record =
    record_like<Record> && UPD_ALL_OF_CONCEPT(ith_record_element_record_like, Record, record_size_v<Record>);

} // namespace upd
