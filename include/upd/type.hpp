//! \file

#pragma once

namespace upd {

//! \brief Portable type for inspecting object representation
//! \details
//!   unsigned char being relevant to inspecting object representation regardless of the platform is enforced by the
//!   standard (<a href="https://en.cppreference.com/w/cpp/language/types#Character_types">Fundamental types - Character
//! Types - cppreference</a>).
using byte_t = unsigned char;

} // namespace upd
