#include "unaligned_data.hpp"

void run_unaligned_data_ut() {
  using namespace upd;

  // Template instantiation check
  {
    byte_t raw_data[16]{};

    unaligned_data<16, endianess::LITTLE, signed_mode::TWO_COMPLEMENT>{};
    unaligned_data<16>{};

    make_unaligned_data<endianess::LITTLE, signed_mode::TWO_COMPLEMENT>(raw_data);
    make_unaligned_data(raw_data);
  }

  // Check every combination of endianess and signed representation
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::SIGNED_MAGNITUDE,
                                                                     0xbc,
                                                                     0x0a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::SIGNED_MAGNITUDE,
                                                                     0xbc,
                                                                     0x8a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::SIGNED_MAGNITUDE,
                                                                     0x0a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::SIGNED_MAGNITUDE,
                                                                     0x8a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::ONE_COMPLEMENT,
                                                                     0xbc,
                                                                     0x0a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -2,
                                                                     endianess::LITTLE,
                                                                     signed_mode::ONE_COMPLEMENT,
                                                                     0xfd,
                                                                     0xff>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::ONE_COMPLEMENT,
                                                                     0x0a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -2,
                                                                     endianess::BIG,
                                                                     signed_mode::ONE_COMPLEMENT,
                                                                     0xff,
                                                                     0xfd>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::TWO_COMPLEMENT,
                                                                     0xbc,
                                                                     0x0a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -126,
                                                                     endianess::LITTLE,
                                                                     signed_mode::TWO_COMPLEMENT,
                                                                     0x82,
                                                                     0xff>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::TWO_COMPLEMENT,
                                                                     0x0a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -126,
                                                                     endianess::BIG,
                                                                     signed_mode::TWO_COMPLEMENT,
                                                                     0xff,
                                                                     0x82>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::OFFSET_BINARY,
                                                                     0xbc,
                                                                     0x8a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::OFFSET_BINARY,
                                                                     0x44,
                                                                     0x75>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::OFFSET_BINARY,
                                                                     0x8a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::OFFSET_BINARY,
                                                                     0x75,
                                                                     0x44>));

  unaligned_data_DO_serialize_data_EXPECT_correct_value_when_unserializing_multiopt(every_options);
  unaligned_data_DO_iterate_throught_content_EXPECT_correct_raw_data_multiopt(every_options);
}
