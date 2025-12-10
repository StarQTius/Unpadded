#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "implementation_of.hpp"
#include "record/lite_record.hpp"
#include "record/record_size.hpp"
#include "record/record_tag.hpp"
#include "static_assert.hpp"
#include "upd.hpp"
#include "variadic/position_of.hpp"
#include "with_sequence.hpp"

namespace upd {

template<typename>
struct tuple_like_for;

template<typename>
struct record_like_for;

} // namespace upd

namespace upd::detail {

template<std::size_t I, typename Tuple>
[[nodiscard]] constexpr auto get_from_tuple(Tuple &&t) -> decltype(auto) {
  UPD_STATIC_ASSERT(I < std::tuple_size_v<std::remove_cvref_t<Tuple>>,
                    "`I`({}) must be lesser than tuple size {}",
                    I,
                    std::tuple_size_v<std::remove_cvref_t<Tuple>>);

  UPD_STATIC_ASSERT((requires { get<I>(UPD_FWD(t)); } || implementation_of<Tuple, tuple_like_for>),
                    "Either `get<I>(t)` or `upd::tuple_like_for<Tuple>::get<I>(t)` must be well-formed");

  if constexpr (requires { get<I>(UPD_FWD(t)); }) {
    return get<I>(UPD_FWD(t));
  } else if constexpr (implementation_of<Tuple, tuple_like_for>) {
    using impl_type = upd::tuple_like_for<std::remove_cvref_t<Tuple>>;
    return impl_type::template get<I>(UPD_FWD(t));
  }
}

template<auto Tag, typename Record>
[[nodiscard]] constexpr auto get_from_record(Record &&rec) -> decltype(auto) {
  constexpr auto tags = UPD_WITH_SEQUENCE(Is, record_size_v<Record>) {
    return std::tuple{record_tag<Is, Record>{}...};
  };

  UPD_STATIC_ASSERT((requires { get<Tag>(UPD_FWD(rec)); } || implementation_of<Record, record_like_for>),
                    "Either `get<{}>(rec)` or `upd::record_like_for<Record>` must be well-formed",
                    Tag,
                    Tag);

  if constexpr (requires { get<Tag>(UPD_FWD(rec)); }) {
    return get<Tag>(UPD_FWD(rec));
  } else if constexpr (implementation_of<Record, record_like_for>) {
    using impl_type = upd::record_like_for<std::remove_cvref_t<Record>>;
    auto i = variadic::position_of<Tag>(tags);
    UPD_STATIC_ASSERT(requires { impl_type::template get_ith<i>(UPD_FWD(rec)); }, "`{}` is not a tag of `rec`", tag);

    return impl_type::template get_ith<i>(UPD_FWD(rec));
  }
}

} // namespace upd::detail

namespace upd {

template<auto IOrTag>
constexpr auto get = []<typename T> [[nodiscard]] (T &&x) -> decltype(auto) {
  constexpr auto is_index_like = [] {
    if constexpr (std::integral<decltype(IOrTag)>) {
      return std::in_range<std::size_t>(IOrTag);
    } else if constexpr (std::convertible_to<decltype(IOrTag), std::size_t>) {
      return true;
    } else {
      return false;
    }
  }();

  UPD_STATIC_ASSERT((implementation_of<T, std::tuple_size> && is_index_like || implementation_of<T, record_size>),
                    "`x` must either be a record or a tuple "
                    "(it does not implement `std::tuple_size` or `upd::record_size`)");

  if constexpr (implementation_of<T, std::tuple_size> && is_index_like) {
    constexpr auto i = static_cast<std::size_t>(([] {}, IOrTag));
    return detail::get_from_tuple<i>(UPD_FWD(x));
  } else if constexpr (implementation_of<T, record_size>) {
    return detail::get_from_record<IOrTag>(UPD_FWD(x));
  }
};

} // namespace upd
