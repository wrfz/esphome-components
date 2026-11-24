# Tests

Two kinds of tests, both run without hardware, locally and in GitHub Actions
(`.github/workflows/test.yml`).

## Unit tests (GoogleTest)

Helper classes of `daikin_rotex_uart`:

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

## Host tests (simulated HPSU)

`test/host/` builds the whole component with the ESPHome host platform as a Linux program. Its
UART opens `/tmp/hpsu-uart-pty`, a symlink to a pseudo terminal; on the other side `conftest.py`
plays the HPSU: it answers each request with the frame the test set for that registry, optionally
in two parts or not at all. It speaks the I protocol (`03 40 <registry> <crc>`) or the S protocol
(`02 <registry> <crc>`, fixed response length).

The pytest tests talk to the program through the native API like Home Assistant and check the
values, the log and the requests: values per registry, responses in parts, CRC errors, lost
requests (`test_uart.py`), protocol detection and the S protocol (`test_s_protocol.py`).

```sh
python3 -m venv .venv && . .venv/bin/activate
pip install esphome==2026.11.0 pytest pytest-asyncio
cd test/host
pytest
```

`pytest` must be the one from the venv (the system pytest has no `aioesphomeapi`). The first run
compiles the program, later runs reuse the build. On a network mount put the build on the local
disk and keep Python from writing bytecode next to the sources:

```sh
export ESPHOME_DATA_DIR=/tmp/hpsu-uart-build PYTHONDONTWRITEBYTECODE=1
pytest
```

To skip compiling: `HPSU_HOST_BINARY=<path to program> pytest`.
