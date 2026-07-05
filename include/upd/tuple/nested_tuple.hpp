#pragma once

#include <cstddef>

#include "../utility/variadic_concept.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"

namespace upd {

template<typename Tuple, std::size_t I>
concept ith_tuple_element_tuple_like = tuple_like2<tuple_element_t<I, Tuple>>;

template<typename Tuple>
concept nested_tuple2 =
    tuple_like2<Tuple> && UPD_ALL_OF_CONCEPT(ith_tuple_element_tuple_like, Tuple, tuple_size_v<Tuple>);

} // namespace upd
