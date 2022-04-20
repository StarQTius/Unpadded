//! \file

#pragma once

#include "unevaluated.hpp"

namespace upd {

//! \brief Available restrictions for action storage
enum class action_features { STATIC_STORAGE_DURATION_ONLY, ANY };

//! \brief Value holder to help deduce action features
//! \tparam Action_Features Features to hold
template<action_features Action_Features>
using action_features_h = unevaluated<action_features, Action_Features>;

namespace policy {

//! \brief Ensures that stored actions have static storage duration
//!
//! In that case, the containing object may refer to the actions callbacks rather than managing memory space for them.
constexpr action_features_h<action_features::STATIC_STORAGE_DURATION_ONLY> static_storage_duration_only;

//! \brief Allows any kind of action to be stored
constexpr action_features_h<action_features::ANY> any_action;

} // namespace policy
} // namespace upd
