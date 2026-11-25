"""Host tests: the component runs as a Linux program, the test plays the HPSU on the serial line.

The firmware is built from hpsu_uart_host.yaml (ESPHome host platform). Its UART opens PTY_LINK,
a symlink to a pseudo terminal whose other side is the simulated HPSU here: it answers each
request in its protocol with the frame set for that registry. Each test starts a fresh process
and talks to it through the native API like Home Assistant.

Protocols:
- I: request 03 40 <registry> <crc>, response 40 <registry> <length> <payload> <crc>
- S: request 02 <registry> <crc>, response <registry> <payload> <crc> with a fixed length per
  registry (18 bytes, 6 for 0x50 and 0x56)

Set HPSU_HOST_BINARY to use an already built program instead of compiling.
"""

from __future__ import annotations

import asyncio
import math
import os
import re
import shutil
import subprocess
import sys
from collections.abc import Callable
from pathlib import Path

import pytest
import pytest_asyncio
from aioesphomeapi import APIClient

HERE = Path(__file__).parent
YAML = HERE / "hpsu_uart_host.yaml"
PTY_LINK = Path("/tmp/hpsu-uart-pty")  # uart: port in hpsu_uart_host.yaml
API_PORT = 16054
TIMEOUT_S = 20.0
UNKNOWN = "unknown"  # state without a value (not read yet, or NaN)

ANSI = re.compile(r"\x1b\[[0-9;]*m")


def crc(data: bytes) -> int:
    """Inverted byte sum, as in TRequest::getCRC."""
    return ~sum(data) & 0xFF


def i_frame(registry: int, payload: bytes) -> bytes:
    """I-protocol response: 40 <registry> <length> <payload> <crc>; length counts from the registry."""
    head = bytes([0x40, registry, len(payload) + 2]) + payload
    return head + bytes([crc(head)])


S_FRAME_LENGTH = {0x50: 6, 0x56: 6}  # all others 18


def s_frame(registry: int, payload: bytes = b"") -> bytes:
    """S-protocol response: <registry> <payload, zero padded to the fixed length> <crc>."""
    size = S_FRAME_LENGTH.get(registry, 18) - 2
    assert len(payload) <= size
    head = bytes([registry]) + payload + bytes(size - len(payload))
    return head + bytes([crc(head)])


def u16_256(value: float) -> bytes:
    """Unsigned little endian value in 1/256 (S protocol temperatures, e.g. 29.5 °C -> 7552)."""
    return int(round(value * 256)).to_bytes(2, "little")


def le16(value: float, divider: float = 10) -> bytes:
    """Signed little endian value as the HPSU sends it (e.g. 29.7 °C -> 297)."""
    return int(round(value * divider)).to_bytes(2, "little", signed=True)


def payload(size: int, **fields: bytes) -> bytes:
    """Payload of `size` zero bytes with fields set at their offsets: payload(12, o2=le16(29.7))."""
    data = bytearray(size)
    for key, value in fields.items():
        offset = int(key[1:])
        data[offset:offset + len(value)] = value
    return bytes(data)


def tv_tr(tv: float, tr: float) -> bytes:
    """Frame of registry 0x61 with Tv and Tr."""
    return i_frame(0x61, payload(12, o2=le16(tv), o8=le16(tr)))


def default_frames() -> dict[int, bytes]:
    """A heating HPSU: Tv 29.7 °C, Tr 26.7 °C, outside 1.5 °C, ..."""
    return {
        0x00: bytes.fromhex("40 00 0E 04 01 00 01 01 01 00 02 01 01 04 39 68"),  # equipment of a Rotex HPSU
        0x10: i_frame(0x10, payload(16, o6=le16(18.7, 10 * 2.56))),  # tgt_evap_temp (raw / 2.56 in tenths)
        0x20: i_frame(0x20, payload(17, o0=le16(1.5))),              # outdoor_temp
        0x21: i_frame(0x21, payload(6, o0=le16(3.5))),               # inv_prim_current
        0x30: i_frame(0x30, payload(12, o1=bytes([81]))),            # fan_speed (x10 rpm on ERGA08EAV3)
        0x60: i_frame(0x60, payload(13, o2=bytes([0x10]))),          # mode_of_operating: heating (high nibble)
        0x61: tv_tr(29.7, 26.7),
        0x62: i_frame(0x62, payload(17, o8=bytes([0x04]))),          # circulation_pump: on (bit 2)
    }


def s_frames() -> dict[int, bytes]:
    """A heating Rotex HPSU on the S protocol (e.g. RDLQ014AA6V3)."""
    return {
        0x53: s_frame(0x53, bytes([0x01])),                                     # circulation pump on
        0x54: s_frame(0x54, payload(16, o2=u16_256(26.5), o4=u16_256(29.5))),  # tr, tv
        0x55: s_frame(0x55, bytes([0x01])),                                     # operation mode: heating
    }


class Hpsu:
    """The simulated HPSU on the master side of the pseudo terminal.

    It speaks `protocol` ("I" or "S"). A request in the other protocol is answered with
    `foreign_reply` (None: no answer; 15 EA is what a device sends for a request it does not know).
    """

    def __init__(self) -> None:
        self.master, self.slave = os.openpty()
        os.set_blocking(self.master, False)
        if PTY_LINK.is_symlink() or PTY_LINK.exists():
            PTY_LINK.unlink()
        PTY_LINK.symlink_to(os.ttyname(self.slave))
        self.protocol = "I"
        self.foreign_reply: bytes | None = None
        self.foreign_requests: list[bytes] = []  # requests in the other protocol
        self.frames = default_frames()   # registry -> response; missing registry: no answer
        self.split: dict[int, int] = {}  # registry -> bytes sent first, the rest on continue_split()
        self.requests: list[int] = []    # registries requested, in order
        self.changed = asyncio.Event()
        self._rx = bytearray()
        self._pending: dict[int, bytes] = {}
        asyncio.get_running_loop().add_reader(self.master, self._on_readable)

    def _on_readable(self) -> None:
        try:
            self._rx += os.read(self.master, 256)
        except OSError:
            return
        while self._rx:
            if self._rx[0] == 0x03 and len(self._rx) >= 4 and self._rx[1] == 0x40:
                request, self._rx = bytes(self._rx[:4]), self._rx[4:]
                protocol, registry = "I", request[2]
            elif self._rx[0] == 0x02 and len(self._rx) >= 3:
                request, self._rx = bytes(self._rx[:3]), self._rx[3:]
                protocol, registry = "S", request[1]
            elif self._rx[0] in (0x02, 0x03):
                break  # request not complete yet
            else:
                del self._rx[0]
                continue
            if request[-1] != crc(request[:-1]):
                continue
            if protocol != self.protocol:
                self.foreign_requests.append(request)
                if self.foreign_reply is not None:
                    os.write(self.master, self.foreign_reply)
                continue
            self.requests.append(registry)
            self._answer(registry)
        self.changed.set()

    def _answer(self, registry: int) -> None:
        frame = self.frames.get(registry)
        if frame is None:
            return
        if (first := self.split.get(registry)) is not None:
            os.write(self.master, frame[:first])
            self._pending[registry] = frame[first:]
        else:
            os.write(self.master, frame)

    def continue_split(self, registry: int) -> None:
        """Send the rest of a response that was sent in parts."""
        os.write(self.master, self._pending.pop(registry))

    def close(self) -> None:
        asyncio.get_running_loop().remove_reader(self.master)
        os.close(self.master)
        os.close(self.slave)
        if PTY_LINK.is_symlink():
            PTY_LINK.unlink()


@pytest.fixture(scope="session")
def binary() -> Path:
    if path := os.environ.get("HPSU_HOST_BINARY"):
        return Path(path)
    esphome = shutil.which("esphome") or str(Path(sys.executable).parent / "esphome")
    subprocess.run([esphome, "compile", str(YAML)], cwd=HERE, check=True)
    # ESPHOME_DATA_DIR moves the build output elsewhere (e.g. off a network mount)
    data_dir = Path(os.environ.get("ESPHOME_DATA_DIR", HERE / ".esphome"))
    return data_dir / "build/hpsu-uart-host/.pioenvs/hpsu-uart-host/program"


class Device:
    """Running host program plus API client; every wait is event driven with a timeout."""

    def __init__(self, process: asyncio.subprocess.Process, hpsu: Hpsu) -> None:
        self.process = process
        self.hpsu = hpsu
        self.client: APIClient | None = None
        self.log: list[str] = []                  # every output line of the program
        self.states: dict[str, str] = {}          # entity name -> current state as text
        self.history: list[tuple[str, str]] = []  # every state received, in order
        self._names: dict[int, str] = {}
        self._changed = asyncio.Event()
        self._reader = asyncio.create_task(self._read_output())

    async def _read_output(self) -> None:
        assert self.process.stdout is not None
        while line := await self.process.stdout.readline():
            self.log.append(ANSI.sub("", line.decode(errors="replace")).rstrip())
            self._changed.set()

    async def wait_for(self, predicate: Callable[[], bool], what: str, timeout: float = TIMEOUT_S) -> None:
        async def loop() -> None:
            while not predicate():
                self._changed.clear()
                self.hpsu.changed.clear()
                waits = [asyncio.create_task(self._changed.wait()), asyncio.create_task(self.hpsu.changed.wait())]
                _, pending = await asyncio.wait(waits, return_when=asyncio.FIRST_COMPLETED)
                for task in pending:
                    task.cancel()

        try:
            await asyncio.wait_for(loop(), timeout)
        except TimeoutError:
            pytest.fail(f"timeout waiting for {what}; states={self.states}; log tail={self.log[-15:]}")

    async def connect(self) -> None:
        # the component logs its request table every second once running; the API is up by then
        await self.wait_for(lambda: any("count:" in line for line in self.log), "component running")
        self.client = APIClient("127.0.0.1", API_PORT, None)
        await self.client.connect(login=True)
        entities, _ = await self.client.list_entities_services()
        self._names = {entity.key: entity.name for entity in entities}
        self.client.subscribe_states(self._on_state)

    def _on_state(self, state) -> None:
        name = self._names.get(state.key)
        if name is None:
            return
        value = state.state
        if getattr(state, "missing_state", False) or (isinstance(value, float) and math.isnan(value)):
            text = UNKNOWN
        elif isinstance(value, float):
            text = f"{value:.1f}"
        else:
            text = str(value)
        self.states[name] = text
        self.history.append((name, text))
        self._changed.set()

    async def wait_state(self, name: str, value: str) -> None:
        await self.wait_for(lambda: self.states.get(name) == value, f"{name} == {value}")

    async def wait_log(self, start: int, text: str) -> None:
        await self.wait_for(lambda: any(text in line for line in self.log[start:]), f"log '{text}'")

    async def wait_requests(self, registry: int, count: int = 1) -> None:
        start = len(self.hpsu.requests)
        await self.wait_for(lambda: self.hpsu.requests[start:].count(registry) >= count,
                            f"{count} request(s) for 0x{registry:02X}")

    async def close(self) -> None:
        if self.client is not None:
            await self.client.disconnect(force=True)
        if self.process.returncode is None:
            self.process.terminate()
            try:
                await asyncio.wait_for(self.process.wait(), 5)
            except TimeoutError:
                self.process.kill()
                await self.process.wait()
        self._reader.cancel()


@pytest_asyncio.fixture
async def hpsu():
    sim = Hpsu()
    try:
        yield sim
    finally:
        sim.close()


@pytest_asyncio.fixture
async def start_device(binary: Path, tmp_path: Path, hpsu: Hpsu):
    """Starts the program when called: a test sets up hpsu.frames before, when it needs them from the start."""
    devices: list[Device] = []

    async def start() -> Device:
        env = {**os.environ, "ESPHOME_PREFDIR": str(tmp_path)}  # fresh preferences for each test
        process = await asyncio.create_subprocess_exec(
            str(binary), stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.STDOUT, env=env
        )
        dev = Device(process, hpsu)
        devices.append(dev)
        await dev.connect()
        return dev

    try:
        yield start
    finally:
        for dev in devices:
            await dev.close()


@pytest_asyncio.fixture
async def device(start_device) -> Device:
    """The program, started with the default frames."""
    return await start_device()
