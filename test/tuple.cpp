#include "tuple.hpp"

int main() {
  using namespace upd;

  // Template instantiation check
  {
    tuple<endianess::LITTLE, signed_mode::TWO_COMPLEMENT, int, char, bool>{0, 0, 0};

    make_tuple(little_endian, two_complement, int{}, char{}, bool{});
    make_tuple(int{}, char{}, bool{});

#if __cplusplus >= 201703L
    tuple{int{}, char{}, bool{}};
#endif
  }

  tuple_DO_set_value_EXPECT_same_value_with_get_multiopt(every_options);
  tuple_DO_set_array_EXPECT_same_value_with_get_multiopt(every_options);
  tuple_DO_iterate_throught_content_EXPECT_correct_raw_data_multiopt(every_options);
  tuple_DO_access_like_array_EXPECT_correct_raw_values_multiopt(every_options);
  tuple_DO_invoke_function_EXPECT_correct_behavior_multiopt(every_options);
  tuple_DO_make_empty_tuple_EXPECT_valid_object_multiopt(every_options);

  UNITY_BEGIN();
  RUN_TEST(tuple_DO_construct_tuple_with_ctad_EXPECT_tuple_holds_correct_values_cpp17);
  RUN_TEST(tuple_DO_bind_names_to_tuple_element_EXPECT_getting_same_values_cpp17);
  RUN_TEST(tuple_DO_serialize_user_provided_structure_EXCEPT_correct_behavior);
  RUN_TEST(tuple_view_DO_iterate_subview_EXPECT_exact_subsequence);
  return UNITY_END();
}
