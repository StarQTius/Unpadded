#pragma once

namespace upd {

template<typename>
concept serializer = true;

template<serializer Serializer>
using byte_type = typename Serializer::byte_type;

} // namespace upd
