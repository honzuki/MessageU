"""A simple implementation of rw-lock

An read-write lock is a lock that allows
multiple readers, but only one writer.

Example:
lock = RWLock()

with lock.reader():
    pass

with lock.writer():
    pass
"""

import threading
from contextlib import contextmanager


class RWLock():
    """Read-Write lock

    This implementation doesn't bias toward anyone.
    """
    def __init__(self):
        self._waiting_lock = threading.Lock()
        self._reader_lock = threading.Lock()
        self._writer_lock = threading.Lock()
        self._reader_count = 0

    def lock_reader(self) -> None:
        """A reader lock

        Allows multiple readers, as long as
        no writer waits for writing.
        Sleeps until it acquires the lock
        """
        with self._waiting_lock:
            with self._reader_lock:
                self._reader_count += 1
                if self._reader_count == 1:
                    self._writer_lock.acquire()

    def release_reader(self) -> None:
        """Releases one reader lock

        doesn't sleep.
        """
        with self._reader_lock:
            self._reader_count -= 1
            if not self._reader_count:
                self._writer_lock.release()

    @contextmanager
    def reader(self):
        """A safe-to-use wrapper around the reader lock

        Example:
        with rwlock.reader():
            pass
        """
        try:
            yield self.lock_reader()
        finally:
            self.release_reader()

    def lock_writer(self) -> None:
        """A write lock

        Only one writer can acquire this lock
        at any given moment.
        Sleeps until it acquires the lock
        """
        with self._waiting_lock:
            self._writer_lock.acquire()

    def release_writer(self) -> None:
        """Releases the writer lock

        doesn't sleep.
        """
        self._writer_lock.release()

    @contextmanager
    def writer(self):
        """A safe-to-use wrapper around the writer lock

        Example:
        with rwlock.writer():
            pass
        """
        try:
            yield self.lock_writer()
        finally:
            self.release_writer()
