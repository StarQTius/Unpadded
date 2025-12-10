#pragma once

#include <concepts>
#include <cstddef>

#include "../get.hpp"
#include "../transfert_reference.hpp"
#include "../upd.hpp"
#include "../variadic_concept.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"

namespace upd {

template<typename Tuple, std::size_t I>
concept ith_tuple_element_owner = requires(Tuple t) {
  { get<I>(UPD_FWD(t)) } -> std::same_as<transfert_reference_t<tuple_element_t<I, Tuple>, Tuple &&>>;
};

template<typename Tuple>
concept regular_tuple = tuple_like2<Tuple> && UPD_ALL_OF_CONCEPT(ith_tuple_element_owner, Tuple, tuple_size_v<Tuple>);

} // namespace upd
