"""Sqlite3 implementation of the database interface

All data in the database is saved according
to the server protocol orientation.

Example:
db = Sqlite3Engine('<file_name>')
db.create_client(username, public_key)
"""

from typing import Iterator, List
from io import BytesIO

import sqlite3
import textwrap
import uuid
import tempfile

from database import engine_interface
from database import types as db_types
from protocol import types as pt_types
import config


class Sqlite3Engine(engine_interface.Database):
    """The sqlite3 engine"""
    def __init__(self, database_name):
        super().__init__()
        self._conn = sqlite3.connect(database_name, check_same_thread=False)
        self._setup()

    def create_client(self, username: pt_types.Username,
                      public_key: pt_types.PublicKey) -> pt_types.ClientID:
        # It doesn't matter if the protocol changes the orientation
        # of this client_id, since we just created it, randomly.
        client_id = pt_types.ClientID.read(BytesIO(uuid.uuid4().bytes))
        with self._lock.writer():
            try:
                self._conn.execute(
                    'INSERT INTO clients(id, username, public_key) VALUES (?, ?, ?)',
                    (
                        client_id.write(),
                        username.write(),
                        public_key.write(),
                    ),
                )
            except sqlite3.IntegrityError as err:
                raise ValueError(
                    'can not create 2 clients with the same username (%s)' %
                    username) from err
            finally:
                self._conn.commit()
        return client_id

    def fetch_client(self, client_id: pt_types.ClientID) -> db_types.Client:
        with self._lock.reader():
            cur = self._conn.execute(
                'SELECT username, public_key, last_seen FROM clients WHERE id=(?)',
                (client_id.write(), ),
            )
            try:
                username, public_key, last_seen = cur.fetchone()
            except TypeError as err:
                raise ValueError('client does not exist') from err

        return db_types.Client(
            client_id,
            pt_types.Username.read(BytesIO(username)),
            pt_types.PublicKey.read(BytesIO(public_key)),
            last_seen,
        )

    def get_client_list(
        self,
        chunk_size: int = config.DB_CHUNK_SIZE
    ) -> Iterator[List[db_types.Client]]:
        assert chunk_size > 0, "can't return chunks of negative amount of rows"

        with self._lock.reader():
            cur = self._conn.execute(
                'SELECT id, username, public_key, last_seen FROM clients')
        while True:
            # We need to lock before reading a new result.
            # This pattern prevent locking the database for
            # a long time, in case the client list is huge,
            # and/or the one using the generator takes his time
            # between sequentives calls to this generator.
            with self._lock.reader():
                result = cur.fetchmany(chunk_size)
            if not result:
                break
            # Convert the row-data from the db into proper Client structures, and yield them
            yield [
                db_types.Client(
                    pt_types.ClientID.read(BytesIO(client_id)),
                    pt_types.Username.read(BytesIO(username)),
                    pt_types.PublicKey.read(BytesIO(public_key)),
                    last_seen,
                ) for client_id, username, public_key, last_seen in result
            ]

    def update_last_seen(self, client: db_types.Client) -> None:
        """Updates the last_seen column to 'now'"""
        with self._lock.writer():
            with self._conn:
                self._conn.execute(
                    "UPDATE clients SET last_seen = datetime('now') WHERE id=(?)",
                    (client.client_id.write(), ),
                )

    def create_message(self, sender: db_types.Client,
                       receiver: db_types.Client,
                       message_type: pt_types.MessageType,
                       content: pt_types.MessageContent) -> pt_types.MessageID:
        # Note that the db saves the content directly into the database,
        # this can be an expensive operation, and considering that it's
        # encrypted anyway (we can't process any valuable query on it),
        # we should prefer saving it to a persistent storage, and only the path
        # in the database.
        # I've tried to abstract it as much as possible, so one would only need
        # to edit this sqlite3 engine in order to implement this change.
        # However, I didn't implement it because we are not allowed to change
        # the database schema for this project. While I could use the blob in
        # a hacky way in order to implement it, it seems like I'll lose points for this.
        with self._lock.writer():
            with self._conn:
                try:
                    cur = self._conn.execute(
                        'INSERT INTO messages(from_id, to_id, type, content) VALUES (?, ?, ?, ?)',
                        (
                            sender.client_id.write(),
                            receiver.client_id.write(),
                            message_type.value,
                            content.write(),  # <- extremely inefficient
                        ),
                    )
                except sqlite3.InterfaceError as err:
                    # Another problem with saving the content
                    # as a blob in the database, instead of a path.
                    raise OverflowError('content size is too big') from err
                return pt_types.MessageID(cur.lastrowid)

    def get_messages(
        self,
        receiver: db_types.Client,
        chunk_size: int = 1  # because the content can be huge
    ) -> Iterator[List[db_types.Message]]:
        assert chunk_size > 0, "can't return chunks of negative amount of rows"
        with self._lock.reader():
            cur = self._conn.execute(
                textwrap.dedent("""
                    SELECT messages.id, type, content, from_id, username, public_key, last_seen 
                    FROM clients, messages 
                    WHERE from_id = clients.id AND to_id=(?)
                """),
                (receiver.client_id.write(), ),
            )
        while True:
            # We need to lock before reading a new result.
            # This pattern prevent locking the database for
            # a long time, in case the message-list is huge,
            # and/or the one using the generator takes his time
            # between sequentives calls to this generator.
            with self._lock.reader():
                result = cur.fetchmany(chunk_size)
            if not result:
                break
            # Convert the row-data from the db into proper Message structures, and yield them
            parsed_result = []
            for (message_id, message_type, raw_content, sender_id,
                 sender_username, sender_public_key,
                 sender_last_seen) in result:
                # we can use the built-in read function, but
                # there's no reason to build the temp-file in chunks.
                temp_file = tempfile.TemporaryFile()
                temp_file.write(raw_content)
                content = pt_types.MessageContent(temp_file)
                sender = db_types.Client(
                    pt_types.ClientID.read(BytesIO(sender_id)),
                    pt_types.Username.read(BytesIO(sender_username)),
                    pt_types.PublicKey.read(BytesIO(sender_public_key)),
                    sender_last_seen,
                )
                message = db_types.Message(
                    pt_types.MessageID(message_id),
                    sender,
                    receiver,
                    pt_types.MessageType(message_type),
                    content,
                )
                parsed_result.append(message)
            yield parsed_result

    def delete_messages(self,
                        message_ids: Iterator[pt_types.MessageID]) -> None:
        with self._lock.writer():
            with self._conn:
                self._conn.executemany(
                    'DELETE FROM messages WHERE id=(?)',
                    ((id.value, ) for id in message_ids),
                )

    def _setup(self) -> None:
        """Initializes the tables if necessary"""
        with self._conn:
            # Clients table
            self._conn.execute(
                textwrap.dedent("""
                    CREATE TABLE IF NOT EXISTS clients(
                        id varchar(%i) NOT NULL,
                        username varchar(%i) NOT NULL UNIQUE,
                        public_key varchar(%i) NOT NULL,
                        last_seen timestamp DEFAULT CURRENT_TIMESTAMP,
                        PRIMARY KEY(id)
                    )
                """ % (
                    pt_types.ClientID.SIZE,
                    pt_types.Username.SIZE,
                    pt_types.PublicKey.SIZE,
                )), )
            # Messages table
            self._conn.execute(
                textwrap.dedent("""
                    CREATE TABLE IF NOT EXISTS messages(
                        id integer PRIMARY KEY AUTOINCREMENT,
                        from_id varchar(%i) NOT NULL,
                        to_id varchar(%i) NOT NULL,
                        type int NOT NULL,
                        content blob NOT NULL,
                        FOREIGN KEY(from_id) REFERENCES clients(id),
                        FOREIGN KEY(to_id) REFERENCES clients(id)
                    )
                """ % (
                    pt_types.ClientID.SIZE,
                    pt_types.ClientID.SIZE,
                )), )
