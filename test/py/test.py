import os
from asyncio import get_event_loop
from pathlib import Path
from random import getrandbits
from shutil import which

import pytest

import unpadded as upd

if which("ccache") is not None:
    os.environ["CC"] = "ccache " + os.environ["CC"]
    os.environ["CXX"] = "ccache " + os.environ["CXX"]

upd.set_extra_include_dirs(["../../include"])
for file in Path(__file__).parent.glob("module*.so"):
    os.remove(file)
pytest_plugins = ("pytest_asyncio",)

from module import *


def test_encode_and_decode():
    assert f1.encode() == b"\x00"
    assert f2.encode(0x10) == b"\x01\x10"
    assert f3.encode() == b"\x02"
    assert f4.encode(0x20) == b"\x03\x20"
    assert f5.encode(0x30) == b"\04\x30"
    assert f6.encode(0x40) == b"\x05\x40"

    assert f1.decode(b"\xee") == 0xEE
    assert f2.decode(b"") is None
    assert f3.decode(b"") is None
    assert f4.decode(b"\xff") == 0xFF
    assert f5.decode(b"") is None
    assert f6.decode(b"") is None


@pytest.mark.asyncio
async def test_asynchronous_key():
    class MockClient(upd.Client):
        def new_request(self, payload):
            future = get_event_loop().create_future()
            future.set_result(
                (2 * payload[1]).to_bytes(1, "little") if len(payload) > 1 else b""
            )
            return future

    client = MockClient()
    assert await client.call(f4, 16) == 32


def test_dispatcher_resolve():
    dispatcher = Dispatcher()
    assert dispatcher.resolve(b"\x01\x10\x00") == b"\x20\x00"


def test_dispatcher_replace_with_pyfunction():
    dispatcher = Dispatcher()
    n = getrandbits(14)
    dispatcher.replace(g2, lambda x: 3 * x)
    assert dispatcher.resolve(b"\x01" + n.to_bytes(2, "little")) == (3 * n).to_bytes(
        2, "little"
    )


def test_fill_dispatcher_manually():
    dispatcher = Dispatcher()

    n = getrandbits(15)
    n_bytes = n.to_bytes(2, "little")
    expected = (2 * n).to_bytes(2, "little")

    assert dispatcher.put(1) == PacketStatus.LOADING_PACKET
    assert dispatcher.put(n_bytes[0]) == PacketStatus.LOADING_PACKET
    assert dispatcher.put(n_bytes[1]) == PacketStatus.RESOLVED_PACKET

    assert dispatcher.is_loaded() and dispatcher.get() == expected[0]
    assert dispatcher.is_loaded() and dispatcher.get() == expected[1]
    assert not dispatcher.is_loaded()


@pytest.mark.asyncio
async def test_fill_future_with_non_bytes():
    expected_result = getrandbits(8)

    class MockClient(upd.Client):
        def new_request(self, payload):
            future = get_event_loop().create_future()
            future.set_result(expected_result)
            return future

    client = MockClient()
    assert await client.call(f4, 0) == expected_result


def test_demangle_function_with_templated_parameters():
    import module

    assert hasattr(module, "f7")


def test_feed_dispatcher_invalid_byte_sequence():
    with pytest.raises(ValueError):
        Dispatcher().resolve(b"\xff")


def test_fill_dispatcher_with_resolve_completely():
    assert Dispatcher().resolve_completely(b"\x00\x01\x10\x20\x02\x67\x89") == [
        b"\xff\x00",
        b"\x20\x40",
        b"",
    ]


def test_dispatcher_packet_dropped_with_resolve_completely():
    with pytest.raises(ValueError):
        assert Dispatcher().resolve_completely(b"\x00\x01\x10\x20\xff\x67\x89")
