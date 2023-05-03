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

## Features

- **Header-only**: You can but do not need to use CMake to integrate Unpadded in your own project. Pick the include/ directory, put it anywhere you like inside your project and you are good.
- **Easy to integrate**: The only dependency of Unpadded is the C++ standard library and a standard-compliant C++11 compiler. Some optional dependencies can be enabled to bind Unpadded to Python, but are not required.
- **Easy to interface with any protocol**: Unpadded is not meant to provide protocol implementation right out of the box, because it would be too difficult to support every chip and framework available. Instead, Unpadded provide you with a way to interface it with almost any protocol.
- **Use C++17 features but is C++11 compatible**: This library use some C++17 features in order to improve compilation time and make it easier to use. However, if you are using an old toolchain, you can also use Unpadded with a C++11 compiler
- **Macro free**: If you are compiling with C++17, then Unpadded will not make you use any macro.
- **Works well with bare-metal application**: Unpadded has been designed to work with hardware interrupts.
- **Optional usage of dynamic allocation**: Can't use dynamic allocation? Then Unpadded will only use the stack. For example, the front page example does not make any dynamic allocation.
- **Additional tools for implementing protocols for non-programmable devices**: Sensors and actuators must sometime be controlled through a communication bus. Unpadded provides you with tools that can be used to implement any communication protocol.
- **Minimizes the quantity of data sent**: Since the signatures of the remotely invocable functions are known at compile-time by both side, Unpadded produces very little information when generating packets.
- **Allows to serialize any user-provided type**: Unpadded natively supports primitive types and arrays of primitive types. Users can define additional serialization rules for any type they want, as long as the number of bytes needed to describe the type is known at compile-time.
- **[Documentation available](https://starqtius.github.io/Unpadded/)**: Albeit a bit sloppy for the moment, every part of the public API is documented. The documentation also includes a step-by-step tutorial.

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
  upd::flist<
    Set_Green_Light,
    Set_Yellow_Light,
    Set_Red_Light>
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
