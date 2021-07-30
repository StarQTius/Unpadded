//! \file
//! \brief Argument forwarding

#pragma once

//! \brief Forward a value
//! \param x forwarded value
#define K2O_FWD(x) static_cast<decltype(x) &&>(x)
