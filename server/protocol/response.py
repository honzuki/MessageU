"""Implementation of all response-structures defined by the MessageU protocol


"""

# pylint: disable=missing-module-docstring
# pylint: disable=missing-class-docstring
# pylint: disable=missing-function-docstring

from __future__ import annotations
from abc import ABC, abstractmethod
from typing import Iterator

from protocol import types

import config


class ResponseSchema(ABC):
    """A scheme for response-structure"""
    @abstractmethod
    def write(self) -> Iterator[bytes]:
        """

        Returns:
            An iterator that yields the response in chunks of raw_data.
        """


class Header(ResponseSchema):
    # The version defines what the protocol supports,
    # hence it's part of the protocol, not a user-defined argument
    VERSION = 2

    def __init__(self, code: types.Code, payload_size: types.PayloadSize):
        self._version = types.Version(self.VERSION)
        self._code = types.Code(code)
        self._payload_size = types.PayloadSize(payload_size)

    def write(self) -> Iterator[bytes]:
        yield (self._version.write() + self._code.write() +
               self._payload_size.write())


class Register(Header):
    CODE = 2100

    def __init__(self, client_id: types.ClientID):
        super().__init__(self.CODE, types.ClientID.SIZE)
        self._client_id = client_id

    def write(self) -> Iterator[bytes]:
        for chunk in super().write():
            yield chunk
        yield self._client_id.write()


class ClientListNode():
    """Represents a single node in the client list"""
    SIZE = types.ClientID.SIZE + types.Username.SIZE

    def __init__(self, client_id: types.ClientID, username: types.Username):
        self._client_id = client_id
        self._username = username

    def write(self) -> Iterator[bytes]:
        yield self._client_id.write() + self._username.write()


class ClientList(Header):
    CODE = 2101

    def __init__(self, payload: Iterator[bytes],
                 payload_size: types.PayloadSize):
        super().__init__(self.CODE, payload_size)
        self._payload = payload

    def write(self) -> Iterator[bytes]:
        for chunk in super().write():
            yield chunk
        for chunk in self._payload:
            yield chunk


class PublicKey(Header):
    CODE = 2102

    def __init__(self, client_id: types.ClientID, public_key: types.PublicKey):
        super().__init__(self.CODE, types.ClientID.SIZE + types.PublicKey.SIZE)
        self._client_id = client_id
        self._public_key = public_key

    def write(self) -> Iterator[bytes]:
        for chunk in super().write():
            yield chunk
        yield self._client_id.write() + self._public_key.write()


class MessageSent(Header):
    CODE = 2103

    def __init__(self, client_id: types.ClientID, message_id: types.MessageID):
        super().__init__(self.CODE, types.ClientID.SIZE + types.MessageID.SIZE)
        self._client_id = client_id
        self._message_id = message_id

    def write(self) -> Iterator[bytes]:
        for chunk in super().write():
            yield chunk
        yield self._client_id.write() + self._message_id.write()


class Message():
    """Represents a single message"""
    HEADER_SIZE = types.ClientID.SIZE + types.MessageID.SIZE + types.MessageType.SIZE

    def __init__(self, sender_id: types.ClientID, message_id: types.MessageID,
                 message_type: types.MessageType,
                 content: types.MessageContent):
        self._sender_id = sender_id
        self._message_id = message_id
        self._message_type = message_type
        self._content = content

    def get_size(self):
        """returns the size of this message"""
        return self.HEADER_SIZE + self._content.size.value

    def write(self) -> Iterator[bytes]:
        yield (self._sender_id.write() + self._message_id.write() +
               self._message_type.write() + self._content.size.write())
        for chunk in self._content.write_by_chunks(config.DATA_CHUNK_SIZE):
            yield chunk


class PendingMessages(Header):
    CODE = 2104

    def __init__(self, payload: Iterator[bytes],
                 payload_size: types.PayloadSize):
        super().__init__(self.CODE, payload_size)
        self._payload = payload

    def write(self) -> Iterator[bytes]:
        for chunk in super().write():
            yield chunk
        for chunk in self._payload:
            yield chunk


class Error(Header):
    CODE = 9000

    def __init__(self):
        super().__init__(self.CODE, 0)
