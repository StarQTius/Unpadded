#include <mutex>
#include <tuple>
#include <type_traits>

#include <upd/detail/variadic/clean.hpp>
#include <upd/detail/variadic/leaf.hpp>

using namespace upd::detail::variadic;

static_assert(std::is_same_v<clean_t<std::tuple<marked_for_cleaning_t, int, bool, std::mutex>>,
                             std::tuple<int, bool, std::mutex>>);

static_assert(std::is_same_v<clean_t<std::tuple<int, marked_for_cleaning_t, bool, std::mutex>>,
                             std::tuple<int, bool, std::mutex>>);

static_assert(std::is_same_v<clean_t<std::tuple<int, bool, marked_for_cleaning_t, std::mutex>>,
                             std::tuple<int, bool, std::mutex>>);

static_assert(std::is_same_v<clean_t<std::tuple<int, bool, std::mutex>>, std::tuple<int, bool, std::mutex>>);

static_assert(std::is_same_v<clean_t<std::tuple<>>, std::tuple<>>);
