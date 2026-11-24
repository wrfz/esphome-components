"""S protocol (older Daikin / Rotex, e.g. RDLQ014AA6V3) and the protocol detection.

Frames and layout as in ESPAltherma (doc/Daikin S protocol.md, include/def/PROTOCOL_S_ROTEX.h):
request 02 <registry> <crc>, response <registry> <payload> <crc> with a fixed length.
"""

import pytest

from conftest import UNKNOWN, payload, s_frame, s_frames, u16_256

pytestmark = pytest.mark.asyncio

I_PROBE = bytes.fromhex("03 40 60 5C")


@pytest.fixture
def s_hpsu(hpsu):
    hpsu.protocol = "S"
    hpsu.frames = s_frames()
    return hpsu


async def test_detects_s_when_i_gets_no_answer(start_device, s_hpsu):
    device = await start_device()

    await device.wait_log(0, "Protocol resolved: S")
    assert I_PROBE in s_hpsu.foreign_requests        # I tried first
    assert s_hpsu.requests[0] == 0x53                # then the S probe


async def test_detects_s_when_i_is_rejected(start_device, s_hpsu):
    # a device answers a request it does not understand with 15 EA: no need to wait for a timeout
    s_hpsu.foreign_reply = bytes.fromhex("15 EA")
    device = await start_device()

    await device.wait_log(0, "Protocol resolved: S")
    assert I_PROBE in s_hpsu.foreign_requests
    resolved = next(i for i, line in enumerate(device.log) if "Protocol resolved: S" in line)
    assert not any("TX[S]" in line for line in device.log[:resolved])  # no S probe needed


async def test_values_reach_home_assistant(start_device, s_hpsu):
    device = await start_device()

    await device.wait_state("Spreizung", "3.0")
    assert device.states["TV"] == "29.5"
    assert device.states["TR"] == "26.5"
    assert device.states["Betriebsart"] == "Heizen"


async def test_circulation_pump(start_device, s_hpsu):
    # PROTOCOL_S_ROTEX.h: 0x53 byte 0 is the circulation pump (0 off, else on)
    device = await start_device()
    await device.wait_state("Umwaelzpumpe", "True")

    s_hpsu.frames[0x53] = s_frame(0x53, bytes([0x00]))
    await device.wait_state("Umwaelzpumpe", "False")


async def test_real_frame_from_a_rotex_rrlq008bav3(start_device, s_hpsu):
    # 0x55 response of an S device (issue wrfz/esphome-components#18): 18 bytes, mode heating
    frame = bytes.fromhex("55 01 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 99")
    assert frame == s_frame(0x55, frame[1:-1])  # length and CRC as the simulator builds them
    s_hpsu.frames[0x55] = frame
    device = await start_device()

    await device.wait_state("Betriebsart", "Heizen")


async def test_dhw_priority(start_device, s_hpsu):
    # PROTOCOL_S_ROTEX.h: 0x53 byte 5 is "Priority to domestic water"
    s_hpsu.frames[0x53] = s_frame(0x53, payload(16, o5=bytes([0x01])))
    device = await start_device()
    await device.wait_state("Vorrang Warmwasser", "True")
    assert device.states["Umwaelzpumpe"] == "False"  # byte 0 is 0

    s_hpsu.frames[0x53] = s_frame(0x53, payload(16, o0=bytes([0x01])))
    await device.wait_state("Vorrang Warmwasser", "False")
    await device.wait_state("Umwaelzpumpe", "True")


async def test_only_registries_with_s_layout_are_requested(start_device, s_hpsu):
    device = await start_device()
    await device.wait_state("Spreizung", "3.0")
    await device.wait_requests(0x54)  # one more round

    assert set(s_hpsu.requests) == {0x53, 0x54, 0x55}
    assert any("TX[S]: 02 54 A9" in line for line in device.log)
    # no S layout: not read on an S device
    assert device.states.get("Aussentemperatur", UNKNOWN) == UNKNOWN


async def test_response_in_parts(start_device, s_hpsu):
    device = await start_device()
    await device.wait_state("TV", "29.5")
    frame = s_frame(0x54, payload(16, o2=u16_256(26.5), o4=u16_256(31.0)))
    s_hpsu.frames[0x54] = frame
    s_hpsu.split[0x54] = 7
    start = len(device.log)

    await device.wait_requests(0x54)
    await device.wait_log(start, "RX[S]: incomplete buffer")
    assert device.states["TV"] == "29.5"

    s_hpsu.continue_split(0x54)
    await device.wait_state("TV", "31.0")
    hex_frame = " ".join(f"{b:02X}" for b in frame)
    assert any(f"RX[S]: {hex_frame}|" in line for line in device.log[start:])


async def test_i_device_is_detected_as_i(device):
    await device.wait_log(0, "Protocol resolved: I")
    assert not device.hpsu.foreign_requests  # never asked in S


async def test_s_only_entity_is_not_read_on_i_device(device):
    # dhw_priority has no I layout: no request for it, no value
    await device.wait_state("Spreizung", "3.0")
    await device.wait_requests(0x61)  # one more round
    assert 0x00 not in device.hpsu.requests
    assert device.states.get("Vorrang Warmwasser", UNKNOWN) == UNKNOWN
    assert not any("count:" in line and " 0x00:" in line for line in device.log)
