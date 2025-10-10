#pragma once

namespace upd {

template<typename Target, typename From>
struct transfert_reference {
  using type = Target;
};

template<typename Target, typename From>
struct transfert_reference<Target, From &> {
  using type = Target &;
};

template<typename Target, typename From>
struct transfert_reference<Target, From &&> {
  using type = Target &&;
};

template<typename Target, typename From>
using transfert_reference_t = typename transfert_reference<Target, From>::type;

} // namespace upd
