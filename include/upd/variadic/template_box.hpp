#pragma once

namespace upd {

template<template<typename...> typename>
void template_box() {}

template<template<auto...> typename>
void template_box() {}

template<template<typename, auto...> typename>
void template_box() {}

} // namespace upd
