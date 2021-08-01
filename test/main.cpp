#include "main.hpp"

void setup() {
  using namespace upd;

  // Template instantiation
  {
    byte_t raw_data[16] {};

    unaligned_data<16, endianess::LITTLE, signed_mode::TWO_COMPLEMENT>{};
    unaligned_data<16>{};

    tuple<endianess::LITTLE, signed_mode::TWO_COMPLEMENT, int, char, bool>{0, 0, 0};

    make_unaligned_data<endianess::LITTLE, signed_mode::TWO_COMPLEMENT>(raw_data);
    make_unaligned_data(raw_data);

    make_tuple<endianess::LITTLE, signed_mode::TWO_COMPLEMENT>(int{}, char{}, bool{});
    make_tuple(int{}, char{}, bool{});

#if __cplusplus >= 201703L
    tuple{int{}, char{}, bool{}};
#endif
  }

  UNITY_BEGIN();

  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, 0xabc,
    endianess::LITTLE, signed_mode::SIGNED_MAGNITUDE,
    0xbc, 0x0a>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, -0xabc,
    endianess::LITTLE, signed_mode::SIGNED_MAGNITUDE,
    0xbc, 0x8a>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, 0xabc,
    endianess::BIG, signed_mode::SIGNED_MAGNITUDE,
    0x0a, 0xbc>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, -0xabc,
    endianess::BIG, signed_mode::SIGNED_MAGNITUDE,
    0x8a, 0xbc>));

  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, 0xabc,
    endianess::LITTLE, signed_mode::ONE_COMPLEMENT,
    0xbc, 0x0a>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, -2,
    endianess::LITTLE, signed_mode::ONE_COMPLEMENT,
    0xfd, 0xff>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, 0xabc,
    endianess::BIG, signed_mode::ONE_COMPLEMENT,
    0x0a, 0xbc>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, -2,
    endianess::BIG, signed_mode::ONE_COMPLEMENT,
    0xff, 0xfd>));

  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, 0xabc,
    endianess::LITTLE, signed_mode::TWO_COMPLEMENT,
    0xbc, 0x0a>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, -126,
    endianess::LITTLE, signed_mode::TWO_COMPLEMENT,
    0x82, 0xff>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, 0xabc,
    endianess::BIG, signed_mode::TWO_COMPLEMENT,
    0x0a, 0xbc>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, -126,
    endianess::BIG, signed_mode::TWO_COMPLEMENT,
    0xff, 0x82>));

  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, 0xabc,
    endianess::LITTLE, signed_mode::OFFSET_BINARY,
    0xbc, 0x8a>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, -0xabc,
    endianess::LITTLE, signed_mode::OFFSET_BINARY,
    0x44, 0x75>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, 0xabc,
    endianess::BIG, signed_mode::OFFSET_BINARY,
    0x8a, 0xbc>));
  RUN_TEST((storage_write_and_inspect_raw_data<
    int16_t, -0xabc,
    endianess::BIG, signed_mode::OFFSET_BINARY,
    0x75, 0x44>));

  storage_write_and_interpret_multiopt(every_options);
  storage_set_and_get_multiopt(every_options);
  storage_set_and_get_array_multiopt(every_options);
  storage_iterate_unaligned_data_multiopt(every_options);
  storage_iterate_tuple_multiopt(every_options);
  storage_access_raw_data_multiopt(every_options);
  storage_invoke_with_tuple_multiopt(every_options);
  storage_make_empty_tuple_multiopt(every_options);

  RUN_TEST(storage_implicitly_convert_array_wrapper_to_array);
  RUN_TEST(storage_construct_tuple_with_parameters_cpp17);
  RUN_TEST(storage_bind_names_to_tuple_element_cpp17);

  UNITY_END();
}

void loop() {}
