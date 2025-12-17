#pragma once

#include <tuple>

#include "concept/invocable.hpp"
#include "upd.hpp"

namespace upd {

template<typename... Args>
struct invoke_following {
public:
  constexpr explicit invoke_following(Args... args) : m_args{UPD_FWD(args)...} {}

  template<typename F>
    requires invocable<F, Args...>
  constexpr auto operator|(F &&f) -> decltype(auto) {
    return std::apply(UPD_FWD(f), UPD_FWD(m_args));
  }

private:
  std::tuple<Args &&...> m_args;
};

template<typename... Args>
invoke_following(Args &&...) -> invoke_following<Args &&...>;

} // namespace upd
