#include <mutex>
#include <tuple>
#include <type_traits>
#include <utility>

#include <upd/detail/variadic/leaf.hpp>
#include <upd/detail/variadic/product.hpp>

using namespace upd::detail::variadic;

static_assert(std::is_same_v<product_t<std::tuple<int, bool, std::mutex>, std::tuple<char, long>>,
                             std::tuple<std::pair<int, char>,
                                        std::pair<bool, char>,
                                        std::pair<std::mutex, char>,
                                        std::pair<int, long>,
                                        std::pair<bool, long>,
                                        std::pair<std::mutex, long>>>);
