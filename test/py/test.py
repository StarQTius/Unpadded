import os
from asyncio import get_event_loop

import pytest

import unpadded as upd

upd.set_extra_include_dirs(["../../include"])

from module import *

pytest_plugins = ("pytest_asyncio",)


class MockClient(upd.Client):
    def new_request(self, payload):
        future = get_event_loop().create_future()
        future.set_result(
            (2 * payload[1]).to_bytes(1, "little") if len(payload) > 1 else b""
        )
        return future


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
    client = MockClient()
    assert await client.call(f4, 16) == 32


def test_dispatcher():
    dispatcher = Dispatcher()
    assert dispatcher.resolve(b"\x01\x10\x00") == b"\x20\00"
