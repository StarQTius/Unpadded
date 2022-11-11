#include <cstdint>
#include <functional>
#include <vector>

#include <Arduino.h>

using namespace fakeit;

int main() {
  When(OverloadedMethod(ArduinoFake(Wire), begin, void(int))).AlwaysReturn();
  When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
  When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();

  std::function<void(int)> on_receive_callback;
  std::vector<int> payload{0, 1, 1, 1, 2, 1, 0, 0, 1, 0, 2, 0};
  size_t i = 0;
  When(OverloadedMethod(ArduinoFake(Wire), read, int())).AlwaysDo([&]() mutable { return payload.at(i++); });
  When(Method(ArduinoFake(Wire), onReceive)).Do([&](auto f) { on_receive_callback = f; });

  setup();
  Verify(Method(ArduinoFake(), pinMode).Using(2, OUTPUT)).Once();
  Verify(Method(ArduinoFake(), pinMode).Using(3, OUTPUT)).Once();
  Verify(Method(ArduinoFake(), pinMode).Using(4, OUTPUT)).Once();

  on_receive_callback(payload.size());

  Verify(Method(ArduinoFake(), digitalWrite).Using(2, HIGH)).Once();
  Verify(Method(ArduinoFake(), digitalWrite).Using(3, HIGH)).Once();
  Verify(Method(ArduinoFake(), digitalWrite).Using(4, HIGH)).Once();
  Verify(Method(ArduinoFake(), digitalWrite).Using(2, LOW)).Once();
  Verify(Method(ArduinoFake(), digitalWrite).Using(3, LOW)).Once();
  Verify(Method(ArduinoFake(), digitalWrite).Using(4, LOW)).Once();

  return 0;
}
