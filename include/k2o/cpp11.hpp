//! \file
//! \brief C++11 utilities

#pragma once

#define K2O_CTREF(VALUE) ::k2o::detail::value_h<decltype(VALUE) &, VALUE>
