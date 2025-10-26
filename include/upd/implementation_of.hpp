#pragma once

#include <type_traits>

namespace upd {

template<typename T, template<typename> typename Traits>
concept implementation_of = requires {
  typename Traits<std::remove_cvref_t<T>>;
  Traits<std::remove_cvref_t<T>>{};
};

} // namespace upd
