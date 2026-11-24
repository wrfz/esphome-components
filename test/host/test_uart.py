"""Reading the HPSU over the serial line (I protocol), with the simulated HPSU from conftest.py."""

import pytest

from conftest import UNKNOWN, i_frame, payload, tv_tr

pytestmark = pytest.mark.asyncio

TV = "TV"
TR = "TR"
SPREAD = "Spreizung"


async def test_values_reach_home_assistant(device):
    await device.wait_state(SPREAD, "3.0")

    assert device.states[TV] == "29.7"
    assert device.states[TR] == "26.7"
    assert device.states["Aussentemperatur"] == "1.5"
    assert device.states["Ziel-Verdampfungstemperatur"] == "18.7"
    assert device.states["INV Primary Current"] == "3.5"
    assert device.states["Ventilatordrehzahl"] == "810.0"   # ERGA08EAV3: fan divider 0.1
    assert device.states["Betriebsart"] == "Heizen"


async def test_request_frames(device):
    await device.wait_state(SPREAD, "3.0")
    assert "TX: 03 40 61 5B" in "\n".join(device.log)
    assert set(device.hpsu.requests) >= {0x10, 0x20, 0x21, 0x30, 0x60, 0x61}


async def test_response_in_parts(device):
    # since ESPHome 2025.10 the ESP32 driver hands over a response in parts (7 bytes at 9600 8E1)
    await device.wait_state(TV, "29.7")
    frame = tv_tr(31.0, 26.7)
    device.hpsu.frames[0x61] = frame
    device.hpsu.split[0x61] = 7
    start = len(device.log)

    await device.wait_requests(0x61)
    await device.wait_log(start, "incomplete buffer")
    assert device.states[TV] == "29.7"  # nothing taken from a part

    device.hpsu.continue_split(0x61)
    await device.wait_state(TV, "31.0")
    # the RX line shows the whole frame, not only the last part
    hex_frame = " ".join(f"{b:02X}" for b in frame)
    assert any(f"RX: {hex_frame}|tv: 31.0" in line for line in device.log[start:])


async def test_crc_mismatch_is_dropped(device):
    await device.wait_state(TV, "29.7")
    bad = bytearray(tv_tr(35.0, 26.7))
    bad[-1] ^= 0xFF
    device.hpsu.frames[0x61] = bytes(bad)
    start = len(device.log)

    await device.wait_requests(0x61)
    await device.wait_log(start, "CRC mismatch")
    assert device.states[TV] == "29.7"

    device.hpsu.frames[0x61] = tv_tr(35.0, 26.7)
    await device.wait_state(TV, "35.0")  # asked again, the next response is taken


async def test_frame_of_another_registry_does_not_set_values(device):
    # a response is matched by its registry: the 0x61 layout sent as 0x62 changes nothing
    await device.wait_state(TV, "29.7")
    device.hpsu.frames[0x61] = i_frame(0x62, payload(12))
    start = len(device.log)

    await device.wait_requests(0x61)
    await device.wait_log(start, "RX: 40 62")
    assert device.states[TV] == "29.7"


async def test_no_response_no_nan_flood(start_device, hpsu):
    # registry 0x61 (tv, tr) never answers: the spread has no value. Before the fix, the smoothed
    # spread published NaN in every loop (about every 100 ms) once the program ran 10 s.
    del hpsu.frames[0x61]
    device = await start_device()

    await device.wait_requests(0x61, count=5)  # asked again each time the request is lost (3 s): > 12 s

    assert device.states.get(TV, UNKNOWN) == UNKNOWN
    published = [state for name, state in device.history if name == SPREAD]
    assert len(published) <= 1, f"{len(published)} states, e.g. {published[:3]}"
