//! \file

#pragma once

#include "unevaluated.hpp"

namespace upd {

//! \brief Available restrictions for action storage
enum class action_features { ANY, WEAK_REFERENCE };

//! \brief Value holder to help deduce action features
//! \tparam Action_Features Features to hold
template<action_features Action_Features>
using action_features_h = unevaluated<action_features, Action_Features>;

namespace policy {

//! \brief Ensures that stored actions refers to free functions or objects with static storage duration
//!
//! In that case, the \ref<action> action may refer to its callback rather than managing memory space for them.
constexpr action_features_h<action_features::WEAK_REFERENCE> weak_reference;

//! \brief Allows any kind of callback to be stored by actions
constexpr action_features_h<action_features::ANY> any_callback;

} // namespace policy
} // namespace upd
