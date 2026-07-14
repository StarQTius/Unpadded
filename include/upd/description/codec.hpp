#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>

#include "../description/codec_info.hpp"
#include "../error.hpp"
#include "../record/record.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/tuple_like.hpp"
#include "../utility/constexpr.hpp"

namespace upd {

template<typename T>
concept codec =
    requires {
      typename T::value_type;
      typename T::input_type;
    }
    && std::default_initializable<typename T::value_type>
    && requires(T x,
                typename T::value_type v,
                typename T::input_type in,
                upd::record<> rec,
                std::tuple<> t,
                stream_interface &st) {
         { x.rules(rec, rec, expr<codec_info{}>) } -> upd::tuple_like2;
         { x.encode(in, st, t) } -> std::convertible_to<result<void>>;
         { v = *x.decode(st, t) };
         { x.bitsize(v) } -> std::convertible_to<std::size_t>;
       };

} // namespace upd
