# Unpadded

Unpadded is a header-only RPC library appropriate for embedded software and compatible with C++11 (with syntactic sugar for C++17).

Unpadded does not require code generation, so it is easy to integrate in any project. It provides compile-time safety mechanisms to avoid silly mistakes while striving for low memory usage.

## What Unpadded is good for and what it cannot do

This library does not provide ready-to-use RPC support by itself. It does not implement any protocol like I2C, SPI, TCP etc... Therefore, one downside of Unpadded is that you must implement a "driver" for the protocol or peripheral you want to use Unpadded with.

Unpadded is best used with protocol that can handle byte streams easily, such as:
- UART configured for sending and receiving 8 bits of data per frame
- UNIX pipes
- TCP protocol
- ...

## Quick example with Arduino

Do note that the Arduino framework is not necessarlily shipped with an implementatio of the standard library, so you might have to provide one yourself if you want to run this example.

In the following example, we have an Arduino board connected through I²C to another Arduino board. The latter is managing a traffic light with its GPIOs.

The project is divided in three files:
- `shared.hpp` which declares the function that can be called on the callee device;
- `caller.cpp` which implements the behavior of the caller device;
- `callee.cpp` which implements the functions that can be called remotely and the dispatching of the caller requests to the correct function.

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
  upd::twos_complement
};
```

### caller.cpp

```cpp
#include <Arduino.h>
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
  gkey(true).write_to(write_byte);
  delay(500);
  Wire.endTransmission();

  Wire.beginTransmission(1);
  ykey(true).write_to(write_byte);
  delay(500);
  Wire.endTransmission();

  Wire.beginTransmission(1);
  rkey(true).write_to(write_byte);
  delay(500);
  Wire.endTransmission();

  Wire.beginTransmission(1);
  gkey(false).write_to(write_byte);
  delay(500);
  Wire.endTransmission();

  Wire.beginTransmission(1);
  ykey(false).write_to(write_byte);
  delay(500);
  Wire.endTransmission();

  Wire.beginTransmission(1);
  rkey(false).write_to(write_byte);
  delay(500);
  Wire.endTransmission();
}
```

### callee.cpp

```cpp
#include <Arduino.h>
#include <Wire.h>

#include <upd/buffered_dispatcher.hpp>

#include "shared.hpp"

static upd::single_buffered_dispatcher dispatcher{keyring, upd::policy::weak_reference};

void Set_Green_Light(std::uint8_t state) {
  digitalWrite(2, state ? HIGH : LOW);
}

void Set_Yellow_Light(std::uint8_t state) {
  digitalWrite(3, state ? HIGH : LOW);
}

void Set_Red_Light(std::uint8_t state) {
  digitalWrite(4, state ? HIGH : LOW);
}

void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  Wire.onReceive([](int n) {
    upd::packet_status status;
    do {
      status = dispatcher.put(Wire.read());
    } while(--n && status != upd::packet_status::DROPPED_PACKET);
  });
  Wire.begin(1);
}

void loop() {}
```
