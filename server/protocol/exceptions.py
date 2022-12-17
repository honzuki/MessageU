"""Internal exceptions for rules the MessageU protocol enforces

"""

from protocol import types


class ProtocolError(RuntimeError):
    """Error interface"""


class MessageTypeError(ProtocolError):
    """Received an unknown message type"""
    def __init__(self, message_type: types.MessageType):
        super().__init__('received a message of unknown type')
        self._message_type = message_type

    def __str__(self):
        return '%s (%s)' % (super(), self._message_type.value)


class MessageSizeMismatch(ProtocolError):
    """The message size does not match the payload size"""
    def __init__(self, expected_size: types.PayloadSize,
                 actual_size: types.PayloadSize):
        super().__init__(
            'expected payload size does not match actual payload size')
        self._expect_size = expected_size
        self._actual_size = actual_size

    def __str__(self):
        return '%s (expected: %s, actual: %s)' % (super(), self._expect_size,
                                                  self._actual_size)
