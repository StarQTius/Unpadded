//! \file
//! \brief C++11 utilities

#pragma once

#define K2O_CTREF(VALUE) ::k2o::detail::unevaluated_value_h<decltype(VALUE) &, VALUE>
