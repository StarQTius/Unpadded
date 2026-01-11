#pragma once

#include <tuple>
#include <utility>

#include "upd.hpp"

namespace upd {

template<typename... Args>
struct invoke_following {
public:
  constexpr explicit invoke_following(Args... args) : m_args{UPD_FWD(args)...} {}

  template<typename F>
  constexpr auto operator|(F &&f) -> decltype(auto) {
    return [&]<auto... Is>(std::index_sequence<Is...>) {
      return UPD_INVOKE(UPD_FWD(f), get<Is>(UPD_FWD(m_args))...);
    }(std::make_index_sequence<sizeof...(Args)>{});
  }

private:
  std::tuple<Args &&...> m_args;
};

template<typename... Args>
invoke_following(Args &&...) -> invoke_following<Args &&...>;

} // namespace upd
