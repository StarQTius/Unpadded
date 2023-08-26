#pragma once

#define GENERATE_VALUES(TYPE, ...) GENERATE(values({(TYPE)__VA_ARGS__}))
