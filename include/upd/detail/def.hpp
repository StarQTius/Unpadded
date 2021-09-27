//! \file
//! \brief Private macro utilities

#define FWD(x) static_cast<decltype(x) &&>(x)
