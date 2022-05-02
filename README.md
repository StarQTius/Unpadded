# Unpadded

Unpadded is a header-only RPC library appropriate for embedded software and compatible with C++11 (with syntactic sugar with C++17).

Unpadded does not require code generation, so it is easy to integrate in any project. Using metaprogramming, the library has been designed to be as close as possible to equivalent C code.

## Quick example with Arduino

Do note that many Arduino framework do not come with a standard library implementation which is needed to compile Unpadded, so the following example should be taken as proof of concept rather than a functional implementation.

In the following example, we have an Arduino board connected through I2C to another Arduino board. The latter is managing a traffic light with its GPIOs.

### shared.hpp

```cpp
#pragma once

#include <cstdint>

#include <upd/keyring.hpp>

void Set_Green_Light(std::uint8_t);
void Set_Yellow_Light(std::uint8_t);
void Set_Red_Light(std::uint8_t);

upd::keyring keyring{
  upd::flist<Set_Green_Light, Set_Yellow_Light, Set_Red_Light>,
  upd::little_endian,
  upd::two_complement
};
```

### caller.cpp

```cpp

#include <Wire.h>

#include "shared.hpp"

void setup() {
  Wire.begin();
}

void loop() {
  auto gkey = keyring.get<Set_Green_Light>();
  auto ykey = keyring.get<Set_Yellow_Light>();
  auto rkey = keyring.get<Set_Red_Light>();

  auto write_byte = [](upd::byte_t x) {
    Wire.write(x);
  };

  Wire.beginTransmission(1);
  gkey(true).write_all(write_byte);
  delay(500);
  Wire.endTransmission();

  Wire.beginTransmission(1);
  ykey(true).write_all(write_byte);
  delay(500);
  Wire.endTransmission();

  Wire.beginTransmission(1);
  rkey(true).write_all(write_byte);
  delay(500);
  Wire.endTransmission();

  Wire.beginTransmission(1);
  gkey(false).write_all(write_byte);
  delay(500);
  Wire.endTransmission();

  Wire.beginTransmission(1);
  ykey(false).write_all(write_byte);
  delay(500);
  Wire.endTransmission();

  Wire.beginTransmission(1);
  rkey(false).write_all(write_byte);
  delay(500);
  Wire.endTransmission();
}

```

### callee.cpp

```cpp

#include <Wire.h>

#include <upd/buffered_dispatcher.hpp>

#include "shared.hpp"

static upd::single_buffered_dispatcher dispatcher{keyring, upd::any_action};

void setup() {
  Wire.onReceive([](int n) {
    upd::packet_status status;
    do {
      status = dispatcher.read([]() { return Wire.read(); });  
    } while(--n && status != upd::packet_status::DROPPED_PACKET);
  });
  Wire.begin(1);
}

void loop() {}

```
