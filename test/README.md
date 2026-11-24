# Tests

Unit tests (GoogleTest) for the helper classes of `daikin_rotex_uart`, run without hardware,
locally and in GitHub Actions (`.github/workflows/test.yml`):

- `BidiMap`, `Utils`, `PID`, `Scheduler`, `UnitConverter`
- `TRequest`: CRC, request frames (I and S protocol), in progress / lost / next request
- `TBuffer`: collecting a response that arrives in parts (since ESPHome 2025.10 the ESP32 UART
  driver hands over received data in parts)

They compile against the headers of a fixed ESPHome release (`ESPHOME_GIT_TAG` in
`CMakeLists.txt`). `fake_uart.h` is a UART bus for the tests, `mock_esphome.cpp` provides
`millis()`, which a test can fix with `esphome::mock::set_millis()`.

Needs `cmake`, a C++20 compiler and `git`:

```sh
cd test
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

On a network mount (sshfs, Samba) use a build folder on the local disk, e.g. `-B /tmp/hpsu-uart-unit-build`.
