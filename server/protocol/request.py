"""Implementation of all request-structures defined by the MessageU protocol


"""

# pylint: disable=missing-module-docstring
# pylint: disable=missing-class-docstring
# pylint: disable=missing-function-docstring

from __future__ import annotations

from io import BytesIO

from protocol import types
from protocol import exceptions

import utils
import config


class Header():
    def __init__(self, client_id: types.ClientID, version: types.Version,
                 code: types.Code, payload_size: types.PayloadSize):
        self.client_id = client_id
        self.version = version
        self.code = code
        self.payload_size = payload_size

    @classmethod
    def read(cls, sock: utils.Socket) -> Header:
        data = BytesIO(
            sock.recv(
                types.ClientID.SIZE + types.Version.SIZE + types.Code.SIZE +
                types.PayloadSize.SIZE,
                True,
            ))
        return Header(
            types.ClientID.read(data),
            types.Version.read(data),
            types.Code.read(data),
            types.PayloadSize.read(data),
        )


class Register():
    CODE = 1100

    def __init__(self, username: types.Username, public_key: types.PublicKey):
        self.username = username
        self.public_key = public_key

    @classmethod
    def read(cls, data: BytesIO) -> Register:
        return Register(
            types.Username.read(data),
            types.PublicKey.read(data),
        )


class ClientList():
    CODE = 1101


class PublicKey():
    CODE = 1102

    def __init__(self, client_id: types.ClientID):
        self.client_id = client_id

    @classmethod
    def read(cls, data: BytesIO) -> PublicKey:
        return PublicKey(types.ClientID.read(data))


class SendMessage():
    CODE = 1103

    def __init__(self, receiver_id: types.ClientID,
                 message_type: types.MessageType,
                 message_content: types.MessageContent):
        self.receiver_id = receiver_id
        self.message_type = message_type
        self.message_content = message_content

    @classmethod
    def read(cls, sock: utils.Socket,
             expected_size: types.PayloadSize) -> SendMessage:
        """
        Args:
            sock: the socket to read from the data
            expected_size: the expected size of the payload

        Raises:
            protocol.exceptions.MessageTypeError: the message type is unknown
            protocol.exceptions.MessageSizeMismatch: the message size is too big / small
        """
        header_size = (types.ClientID.SIZE + types.MessageType.SIZE +
                       types.MessageSize.SIZE)
        data = BytesIO(sock.recv(header_size, True))
        receiver_id = types.ClientID.read(data)
        message_type = types.MessageType.read(data)
        message_size = types.MessageSize.read(data)
        actual_size = types.PayloadSize(header_size + message_size.value)
        if actual_size.value != expected_size.value:
            raise exceptions.MessageSizeMismatch(expected_size, actual_size)
        if message_type.value not in types.MessageType.SUPPORT_VALUES:
            raise exceptions.MessageTypeError(message_type.value)

        content_generator = (sock.recv(chunk_size)
                             for chunk_size in utils.get_chunk_sizes(
                                 message_size.value, config.DATA_CHUNK_SIZE))
        return SendMessage(
            receiver_id,
            message_type,
            types.MessageContent.read(content_generator),
        )


class PendingMessages():
    CODE = 1104
