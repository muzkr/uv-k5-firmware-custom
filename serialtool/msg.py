# Copyright (c) 2025 muzkr
#
#   https://github.com/muzkr
#
# Licensed under the MIT License (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at the root of this repository.
#
#     Unless required by applicable law or agreed to in writing, software
#     distributed under the License is distributed on an "AS IS" BASIS,
#     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#     See the License for the specific language governing permissions and
#     limitations under the License.
#

"""
Serial message functionalities
"""

MSG_NOTIFY_DEV_INFO = 0x0518
MSG_NOTIFY_BL_VER = 0x0530
MSG_PROG_FW = 0x0519
MSG_PROG_FW_RESP = 0x051A
MSG_PROG_f80 = 0x0516
MSG_PROG_f80_RESP = 0x0517

# _MSG_LOG = 0x4C4C  # 'L' 'L'
# _MSG_LOG_OBFUSCATED = 0x205A


class Msg:

    def make(msg_type: int, data_len: int):
        obj = Msg(4 + data_len)
        obj.set_msg_type(msg_type)
        return obj

    def __init__(self, buf: bytearray | int):
        if isinstance(buf, int):
            buf = bytearray(buf)
        self.buf = buf
        self._set_data_len(len(buf) - 4)

    def get_msg_type(self) -> int:
        return _get_hw_LE(self.buf)

    def set_msg_type(self, msg_type: int):
        _put_hw_LE(msg_type, self.buf)

    def get_data_len(self) -> int:
        return _get_hw_LE(self.buf, 2)

    def _set_data_len(self, data_len: int):
        _put_hw_LE(data_len, self.buf, 2)

    def get_hw_LE(self, off: int) -> int:
        return _get_hw_LE(self.buf, off)

    def get_word_LE(self, off: int) -> int:
        return _get_word_LE(self.buf, off)

    def set_hw_LE(self, off: int, n: int):
        _put_hw_LE(n, self.buf, off)

    def set_word_LE(self, off: int, n: int):
        _put_word_LE(n, self.buf, off)


def fetch(buf: bytearray) -> Msg | None:

    if len(buf) < 8:
        return None

    pack_begin = buf.find(b"\xab\xcd")
    if -1 == pack_begin:
        if buf.endswith(b"\xab"):
            del buf[:-1]
        else:
            del buf[:]

        return None

    if len(buf) - pack_begin < 8:
        return None

    msg_len = _get_hw_LE(buf, pack_begin + 2)
    pack_end = pack_begin + 6 + msg_len

    if not buf.startswith(b"\xdc\xba", pack_end):
        # We've got wrong beginning
        del buf[: pack_begin + 2]
        return None

    # --------------
    #  Packet complete

    # Message type 0x4c4c (obfuscated to 0x205a) is for logging
    # msg_type = _get_hw_LE(buf, pack_begin + 4)
    # if _MSG_LOG == msg_type or _MSG_LOG_OBFUSCATED == msg_type:
    #     # Log messages are not obfuscated
    #     pass
    # else:
    #     _obfus(buf, pack_begin + 4, msg_len + 2)

    _obfus(buf, pack_begin + 4, msg_len + 2)

    crc = _get_hw_LE(buf, pack_end - 2)
    msg = Msg(buf[pack_begin + 4 : pack_end - 2])

    del buf[: pack_end + 2]

    # Validate CRC: don't. Messages from device do not apply correct CRC
    crc += 0

    return msg


def make_packet(msg: bytes) -> bytes:

    msg_len = len(msg)
    if 0 != (msg_len % 2):
        msg_len += 1

    buf = bytearray(8 + msg_len)
    _put_hw_LE(0xCDAB, buf)
    _put_hw_LE(msg_len, buf, 2)
    _put_hw_LE(0xBADC, buf, 6 + msg_len)

    with memoryview(buf) as view:
        view[4 : 4 + len(msg)] = msg

    crc = calc_CRC(buf, 4, msg_len)
    _put_hw_LE(crc, buf, 4 + msg_len)
    _obfus(buf, 4, 2 + msg_len)
    return buf


def calc_CRC(buf: bytes, off: int = 0, size: int = 0) -> int:

    CRC = 0

    for i in range(size):

        b = 0xFF & buf[off + i]
        CRC ^= b << 8

        for j in range(8):
            # Check bit [15]
            if 1 & (CRC >> 15):
                CRC = (CRC << 1) ^ 0x1021
            else:
                CRC = CRC << 1

            CRC = 0xFFFF & CRC

    return CRC


_OBFUS_TBL = b"\x16\x6c\x14\xe6\x2e\x91\x0d\x40\x21\x35\xd5\x40\x13\x03\xe9\x80"


def _obfus(buf: bytearray, off: int = 0, size: int = 0):
    N = len(_OBFUS_TBL)
    for i in range(size):
        buf[off + i] ^= _OBFUS_TBL[i % N]


def _get_hw_LE(buf: bytes, off: int = 0) -> int:
    return (buf[off + 1] << 8) | buf[off]


def _get_word_LE(buf: bytes, off: int = 0) -> int:
    return (buf[off + 3] << 24) | (buf[off + 2] << 16) | (buf[off + 1] << 8) | buf[off]


def _put_hw_LE(n: int, buf: bytes, off: int = 0):
    buf[off] = 0xFF & n
    buf[off + 1] = 0xFF & (n >> 8)


def _put_word_LE(n: int, buf: bytes, off: int = 0):
    buf[off] = 0xFF & n
    buf[off + 1] = 0xFF & (n >> 8)
    buf[off + 2] = 0xFF & (n >> 16)
    buf[off + 3] = 0xFF & (n >> 24)
