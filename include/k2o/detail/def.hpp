//! \file
//! \brief Private macro utilities

#define FWD(x) static_cast<decltype(x) &&>(x)
#define STATIC_ASSERT(CONDITION, MSG) static_assert(CONDITION, "\e[1;7m" MSG "\e[1;0m")
