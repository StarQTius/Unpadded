#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "invoke_following.hpp"

#define UPD_WITH_SEQUENCE(IS, N, ...)                                                                                  \
  ::upd::invoke_following{::std::make_index_sequence<N>{}} | [__VA_ARGS__]<::std::size_t... IS>(                       \
                                                                 ::std::index_sequence<IS...>)

static_assert(requires { UPD_WITH_SEQUENCE(Is, 0){}; });

#define UPD_WITH_SEQUENCE_FOR(IS, TUPLE, ...)                                                                          \
  UPD_WITH_SEQUENCE(IS, ::std::tuple_size_v<::std::remove_cvref_t<TUPLE>>, __VA_ARGS__)

static_assert(requires { UPD_WITH_SEQUENCE_FOR(Is, std::tuple<>){}; });
