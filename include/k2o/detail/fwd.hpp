//! \file
//! \brief FWD macro

#pragma once

//! \brief Forward a value
//! \param x forwarded value
#define FWD(x) static_cast<decltype(x) &&>(x)
