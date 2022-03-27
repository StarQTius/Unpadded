//! \file
//! \brief Class behavior tunability

#pragma once

#include <type_traits>

namespace k2o {

//! \brief Available restrictions for order storage
enum class order_features { STATIC_STORAGE_DURATION_ONLY, ANY };

//! \brief Value holder to help deduce order features
template<order_features Order_Features>
using order_features_h = std::integral_constant<order_features, Order_Features>;

namespace policy {

//! \brief Ensures that stored orders have static storage duration
//! In that case, the containing object may refer to orders rather than managing memory space for them.
constexpr order_features_h<order_features::STATIC_STORAGE_DURATION_ONLY> static_storage_duration_only;

//! \brief Allows any kind of order to be stored
constexpr order_features_h<order_features::ANY> any_order;

} // namespace policy
} // namespace k2o
