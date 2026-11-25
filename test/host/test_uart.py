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
    # registry 0x61 (tv, tr) never answers: the spread has no value. The smoothed spread must not
    # publish NaN with every loop.
    del hpsu.frames[0x61]
    device = await start_device()

    await device.wait_requests(0x61, count=5)  # asked again each time the request is lost (3 s): > 12 s

    assert device.states.get(TV, UNKNOWN) == UNKNOWN
    published = [state for name, state in device.history if name == SPREAD]
    assert len(published) <= 1, f"{len(published)} states, e.g. {published[:3]}"


# Frames of a Rotex HPSU
REAL_0X30_HEATING = bytes.fromhex("40 30 0D 3A 50 00 D1 01 00 00 80 00 00 00 A6")   # 4-way valve: byte 7 = 80
REAL_0X30_DEFROST = bytes.fromhex("40 30 0D 2C 00 00 5E 01 00 00 00 00 00 00 F7")   # while defrosting: 00
REAL_0X21 = bytes.fromhex("40 21 07 3A 00 32 00 00 2B")  # 5 payload bytes: no voltage (offset 4, 2 bytes)


async def test_4_way_valve(device):
    device.hpsu.frames[0x30] = REAL_0X30_HEATING
    start = len(device.log)
    await device.wait_state("4-Wege-Ventil", "Heizen")
    await device.wait_state("Ventilatordrehzahl", "800.0")
    await device.wait_log(start, "4_way_valve: Heizen")

    device.hpsu.frames[0x30] = REAL_0X30_DEFROST
    await device.wait_state("4-Wege-Ventil", "Kühlen")


async def test_offset_beyond_payload_gives_no_value(start_device, hpsu):
    hpsu.frames[0x21] = REAL_0X21
    device = await start_device()
    start = 0

    await device.wait_state("INV Primary Current", "5.8")  # the other values of the frame still count
    await device.wait_requests(0x21, count=2)

    assert device.states.get("Spannung", UNKNOWN) == UNKNOWN  # would be byte 4 and the CRC
    warnings = [line for line in device.log[start:] if "voltage: offset 4" in line]
    assert len(warnings) == 1, warnings  # reported once, not with every frame


async def test_binary_value_in_log(device):
    # binary values as on/off in the RX line
    await device.wait_state("Umwaelzpumpe", "True")
    await device.wait_log(0, "circulation_pump: on")


async def test_outdoor_equipment(device):
    # 0x00 of a Rotex HPSU: 1 inverter compressor, 1 fan, 1 expansion valve, 1 4-way valve, 2 solenoid valves
    await device.wait_state("indoor_units_connected", "1.0")
    assert {name: device.states.get(name) for name in (
        "outdoor_inv_compressors", "outdoor_std_compressors", "outdoor_fans", "outdoor_expansion_valves",
        "outdoor_4_way_valves", "outdoor_crankcase_heaters", "outdoor_solenoid_valves", "indoor_units_max",
    )} == {
        "outdoor_inv_compressors": "1.0", "outdoor_std_compressors": "0.0", "outdoor_fans": "1.0",
        "outdoor_expansion_valves": "1.0", "outdoor_4_way_valves": "1.0", "outdoor_crankcase_heaters": "0.0",
        "outdoor_solenoid_valves": "2.0", "indoor_units_max": "1.0",
    }


# 0x60, 0x10 and 0x20 of a Rotex HPSU
HEATING_0X60 = bytes.fromhex("40 60 12 80 00 18 00 40 3E 3C EA 01 4A 01 E0 86 05 03 00 57")
HOT_WATER_0X60 = bytes.fromhex("40 60 12 80 00 48 00 40 3E 3C EA 01 E0 01 E0 C6 05 03 00 51")
STANDBY_0X60 = bytes.fromhex("40 60 12 80 00 00 00 40 3E 3C FA 00 4A 01 20 84 05 03 00 22")
STARTUP_0X10 = bytes.fromhex("40 10 12 01 20 00 00 00 00 70 03 00 00 00 00 00 00 00 00 09")
REAL_0X20 = bytes.fromhex("40 20 13 DD FF 00 00 53 02 00 00 BF FF 00 00 46 00 BF 00 00 98")


async def test_indoor_unit_states(device):
    device.hpsu.frames[0x60] = HEATING_0X60
    await device.wait_state("supply_setpoint", "33.0")
    assert {k: device.states[k] for k in ("dhw_setpoint", "thermostat", "flow_switch", "water_pump", "3_way_valve")} == {
        "dhw_setpoint": "49.0", "thermostat": "True", "flow_switch": "True", "water_pump": "True", "3_way_valve": "Heizen"}

    device.hpsu.frames[0x60] = HOT_WATER_0X60
    await device.wait_state("3_way_valve", "Warmwasserbereitung")
    assert device.states["supply_setpoint"] == "48.0"

    device.hpsu.frames[0x60] = STANDBY_0X60
    await device.wait_state("water_pump", "False")
    assert {k: device.states[k] for k in ("dhw_setpoint", "thermostat", "flow_switch")} == {
        "dhw_setpoint": "25.0", "thermostat": "False", "flow_switch": "False"}


async def test_outdoor_unit_states(device):
    device.hpsu.frames[0x10] = STARTUP_0X10
    device.hpsu.frames[0x20] = REAL_0X20
    await device.wait_state("startup_control", "True")
    await device.wait_state("heat_sink_temp", "7.0")
