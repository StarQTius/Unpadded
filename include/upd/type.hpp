#pragma once

/*!
  \file
  \brief Utility types for portability
*/

namespace upd {

/*!
  \brief Portable type for inspecting object representation
  \details
    unsigned char being relevant to inspecting object representation regardless of the platform is enforced by the
    standard (<a href="https://en.cppreference.com/w/cpp/language/types#Character_types">Fundamental types - Character
  Types - cppreference</a>).
*/
using byte_t = unsigned char;

/*!
  \brief Definition of size_t as specified by the standard
  \details For futher information : <a href="https://en.cppreference.com/w/cpp/types/size_t">std::size_t -
  cppreference</a>.
*/
using size_t = decltype(sizeof(0));

} // namespace upd
