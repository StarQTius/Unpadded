#pragma once

namespace upd {

enum class codec_operation { encoding, decoding };

struct codec_info {
  codec_operation operation;
};

} // namespace upd
