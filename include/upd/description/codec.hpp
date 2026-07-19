#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../description/codec_info.hpp"
#include "../error.hpp"
#include "../record/name.hpp"
#include "../record/record.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/tuple_like.hpp"
#include "../utility/constexpr.hpp"

namespace upd {

template<typename T>
concept codec =
    requires {
      typename std::remove_cvref_t<T>::value_type;
      typename std::remove_cvref_t<T>::input_type;
    }
    && std::default_initializable<typename std::remove_cvref_t<T>::value_type>
    && requires(T x,
                typename std::remove_cvref_t<T>::value_type v,
                typename std::remove_cvref_t<T>::input_type in,
                upd::record<> rec,
                std::tuple<> t,
                stream_interface &st) {
         {
           x.template rules<anon>(rec, rec, expr<codec_info{}>)
         } -> upd::tuple_like2;
         {
           x.template encode<anon>(in, st, t)
         } -> std::convertible_to<result<void>>;
         { v = *x.template decode<anon>(st, t) };
         { x.bitsize(v) } -> std::convertible_to<std::size_t>;
       };

} // namespace upd
