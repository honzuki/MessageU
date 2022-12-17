"""Defines the data-structure that the engine supports

"""

# pylint: disable=missing-module-docstring
# pylint: disable=missing-class-docstring
# pylint: disable=missing-function-docstring

from protocol import types


class Client():
    def __init__(self, client_id: types.ClientID, username: types.Username,
                 public_key: types.PublicKey, last_seen: str):
        self.client_id = client_id
        self.username = username
        self.public_key = public_key
        self.last_seen = last_seen


class Message():
    def __init__(self, message_id: types.MessageID, from_client: Client,
                 to_client: Client, message_type: types.MessageID,
                 content: types.MessageContent):
        self.id = message_id  # it's a good name; pylint: disable=C0103
        self.from_client = from_client
        self.to_client = to_client
        self.type = message_type
        self.content = content
