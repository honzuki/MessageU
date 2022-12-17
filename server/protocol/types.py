"""Implementation of all types defined by the MessageU protocol

"""

# pylint: disable=missing-module-docstring
# pylint: disable=missing-class-docstring
# pylint: disable=missing-function-docstring

from __future__ import annotations
from typing import Iterator
from abc import ABC, abstractmethod

import tempfile
import struct
import io

PROTOCOL_ORIENTATION = '<'


class TypeSchema(ABC):
    """A scheme for a type in the protocol

    Prefer using the read&write methods
    """
    SIZE = 0
    TYPE = ''

    @abstractmethod
    def write(self) -> bytes:
        """Get a raw data representation of the type"""

    @classmethod
    @abstractmethod
    def read(cls, data: io.BytesIO) -> TypeSchema:
        """Parses the raw data into the appropriate type"""


class ClientID(TypeSchema):
    SIZE = 16
    TYPE = '%is' % SIZE

    def __init__(self, value):
        self.value = value

    def write(self) -> bytes:
        return struct.pack(PROTOCOL_ORIENTATION + self.TYPE, self.value)

    def __str__(self) -> str:
        return '%s(%s)' % (self.__class__.__name__, self.value)

    @classmethod
    def read(cls, data: io.BytesIO) -> ClientID:
        (value, ) = struct.unpack(
            PROTOCOL_ORIENTATION + cls.TYPE,
            data.read(cls.SIZE),
        )
        return ClientID(value)


class Version(TypeSchema):
    SIZE = 1
    TYPE = 'B'

    def __init__(self, value):
        self.value = value

    def write(self) -> bytes:
        return struct.pack(PROTOCOL_ORIENTATION + self.TYPE, self.value)

    def __str__(self) -> str:
        return '%s(%s)' % (self.__class__.__name__, self.value)

    @classmethod
    def read(cls, data: io.BytesIO) -> Version:
        (value, ) = struct.unpack(
            PROTOCOL_ORIENTATION + cls.TYPE,
            data.read(cls.SIZE),
        )
        return Version(value)


class Code(TypeSchema):
    SIZE = 2
    TYPE = 'H'

    def __init__(self, value):
        self.value = value

    def write(self) -> bytes:
        return struct.pack(PROTOCOL_ORIENTATION + self.TYPE, self.value)

    def __str__(self) -> str:
        return '%s(%s)' % (self.__class__.__name__, self.value)

    @classmethod
    def read(cls, data: io.BytesIO) -> Code:
        (value, ) = struct.unpack(
            PROTOCOL_ORIENTATION + cls.TYPE,
            data.read(cls.SIZE),
        )
        return Code(value)


class PayloadSize(TypeSchema):
    SIZE = 4
    TYPE = 'L'
    MAX_PAYLOAD_SIZE = 2**(SIZE * 8)

    def __init__(self, value):
        self.value = value

    def write(self) -> bytes:
        return struct.pack(PROTOCOL_ORIENTATION + self.TYPE, self.value)

    def __str__(self) -> str:
        return '%s(%s)' % (self.__class__.__name__, self.value)

    @classmethod
    def read(cls, data: io.BytesIO) -> PayloadSize:
        (value, ) = struct.unpack(
            PROTOCOL_ORIENTATION + cls.TYPE,
            data.read(cls.SIZE),
        )
        return PayloadSize(value)


class Username(TypeSchema):
    SIZE = 255
    TYPE = '%is' % SIZE

    def __init__(self, value):
        self.value = value

    def write(self) -> bytes:
        return struct.pack(PROTOCOL_ORIENTATION + self.TYPE, self.value)

    def __str__(self) -> str:
        return '%s(%s)' % (self.__class__.__name__, self.value)

    @classmethod
    def read(cls, data: io.BytesIO) -> Username:
        (value, ) = struct.unpack(
            PROTOCOL_ORIENTATION + cls.TYPE,
            data.read(cls.SIZE),
        )
        return Username(value)


class PublicKey(TypeSchema):
    SIZE = 160
    TYPE = '%is' % SIZE

    def __init__(self, value):
        self.value = value

    def write(self) -> bytes:
        return struct.pack(PROTOCOL_ORIENTATION + self.TYPE, self.value)

    def __str__(self) -> str:
        return '%s(%s)' % (self.__class__.__name__, self.value)

    @classmethod
    def read(cls, data: io.BytesIO) -> PublicKey:
        (value, ) = struct.unpack(
            PROTOCOL_ORIENTATION + cls.TYPE,
            data.read(cls.SIZE),
        )
        return PublicKey(value)


class MessageID(TypeSchema):
    SIZE = 4
    TYPE = 'I'

    def __init__(self, value):
        self.value = value

    def write(self) -> bytes:
        return struct.pack(PROTOCOL_ORIENTATION + self.TYPE, self.value)

    def __str__(self) -> str:
        return '%s(%s)' % (self.__class__.__name__, self.value)

    @classmethod
    def read(cls, data: io.BytesIO) -> MessageID:
        (value, ) = struct.unpack(
            PROTOCOL_ORIENTATION + cls.TYPE,
            data.read(cls.SIZE),
        )
        return MessageID(value)


class MessageType(TypeSchema):
    SIZE = 1
    TYPE = 'B'
    SUPPORT_VALUES = [1, 2, 3, 4]

    def __init__(self, value):
        self.value = value

    def write(self) -> bytes:
        return struct.pack(PROTOCOL_ORIENTATION + self.TYPE, self.value)

    def __str__(self) -> str:
        return '%s(%s)' % (self.__class__.__name__, self.value)

    @classmethod
    def read(cls, data: io.BytesIO) -> MessageType:
        (value, ) = struct.unpack(
            PROTOCOL_ORIENTATION + cls.TYPE,
            data.read(cls.SIZE),
        )
        return MessageType(value)


class MessageSize(TypeSchema):
    SIZE = 4
    TYPE = 'I'

    def __init__(self, value):
        self.value = value

    def write(self) -> bytes:
        return struct.pack(PROTOCOL_ORIENTATION + self.TYPE, self.value)

    def __str__(self) -> str:
        return '%s(%s)' % (self.__class__.__name__, self.value)

    @classmethod
    def read(cls, data: io.BytesIO) -> MessageSize:
        (value, ) = struct.unpack(
            PROTOCOL_ORIENTATION + cls.TYPE,
            data.read(cls.SIZE),
        )
        return MessageSize(value)


class MessageContent(TypeSchema):
    """A special type of dynamic size, that holds its value in a file.

    This is due to the fact that the content can be huge,
    and the content is supposed to be encrypted anyway.
    """
    def __init__(self, content: io.FileIO):
        content.seek(0, 2)  # Go to the end of the file
        self.size = MessageSize(content.tell())  # Check distance from start
        content.seek(0)  # Return to the start of the file
        self.value = content

    def write(self) -> bytes:
        """Extremley inefficient, prefer using write_by_chunks"""
        return self.value.read()

    def write_by_chunks(self, chunk_size: int) -> Iterator[bytes]:
        """Returns the data in chunks

        Args:
            chunk_size: the maximum amount of bytes in each chunk
        """
        assert chunk_size > 0, "can't return chunks of negative amount of bytes"
        self.value.seek(0)
        while True:
            chunk = self.value.read(chunk_size)
            if not chunk:
                break
            yield chunk

    def __str__(self) -> str:
        return '%s(%s)' % (self.__class__.__name__, self.size)

    @classmethod
    def read(cls, data: Iterator[bytes]) -> MessageContent:
        """The data structure is unknown, we don't parse it in any orientation

        Args:
            data: an iterator that returns sequentives chunks of the raw-bytes.
        """
        content = tempfile.TemporaryFile()
        for chunk in data:
            content.write(chunk)
        # no need to return to the start
        return MessageContent(content)
