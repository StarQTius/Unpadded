#include <cassert>
#include <cstdint>
#include <vector>

#include <Arduino.h>

using namespace fakeit;

int main() {
  When(OverloadedMethod(ArduinoFake(Wire), begin, void())).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(Wire), beginTransmission, void(int))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(Wire), endTransmission, uint8_t())).AlwaysReturn(0);
  When(Method(ArduinoFake(), delay)).AlwaysReturn();

  std::vector<int> payload;
  When(OverloadedMethod(ArduinoFake(Wire), write, size_t(uint8_t))).AlwaysDo([&](uint8_t byte) {
    payload.push_back(byte);
    return 0;
  });

  loop();

  assert((payload == std::vector{0, 1, 1, 1, 2, 1, 0, 0, 1, 0, 2, 0}));

  return 0;
}
