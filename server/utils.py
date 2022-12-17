"""Utilities

Example:
sock = Socket(conn)
sock.recv(50, True)

for chunk in get_chunk_sizes(total, chunk_size):
    pass
"""

from typing import Iterator

import socket


class Socket():
    """Masks the original socket class

    Provides additional, desired, functionality to the
    basic socket class.
    """
    def __init__(self, sock: socket):
        self._base_sock = sock

    def recv(self, count: int, blocking: bool = False) -> bytes:
        """Poll data from the socket connection

        Args:
            count: amount of bytes to poll
            blocking: whether the recv function will block until
                exact amount of bytes were polled from the connection,
                or may return with smaller amount of bytes.

        Returns:
            bytes object that contains the raw data

        Raises:
            EOFError: blocking was requested but can not read exact amount
                of bytes from the stream.
        """
        if blocking:
            data = bytes()
            recv_count = 0
            while recv_count < count:
                new_data = self._base_sock.recv(count - recv_count)
                if not new_data:
                    raise EOFError(
                        'failed to read %i bytes from the socket connection' %
                        (count - recv_count))
                data += new_data
                recv_count = len(data)
            return data
        return self._base_sock.recv(count)

    def send(self, data: bytes) -> None:
        """Send bytes over the connection"""
        return self._base_sock.send(data)


def get_chunk_sizes(total_size: int, chunk_size: int) -> Iterator[int]:
    """Split a size into chunks

    Args:
        total_size: the total size of the data you want to fragment
        chunk_size: the size of each size

    Returns:
        an iterator that yields the size of the next chunk in the sequence.
    """
    size = total_size
    while True:
        if size < chunk_size:
            yield size
            return
        size -= chunk_size
        yield chunk_size
