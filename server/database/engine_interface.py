"""Database integration

Defines a generic interface to the database, that
can even be implemented using memory storage, if desired.
Supports multi-threading by design.

You should never use this directly; use an engine
implementation of this interface design.
"""
from abc import ABC, abstractmethod
from typing import Iterator, List

import config
import rwlock
from database import types as db_types
from protocol import types as pt_types


class Database(ABC):
    """Defines how a database interface should be structured."""
    def __init__(self):
        self._lock = rwlock.RWLock()

    @abstractmethod
    def create_client(self, username: pt_types.Username,
                      public_key: pt_types.PublicKey) -> pt_types.ClientID:
        """Adds a new client to the db

        Returns:
            The id of the newly constructed client

        Raises:
            ValueError: The username is already in use.
        """

    @abstractmethod
    def fetch_client(self, client_id: pt_types.ClientID) -> db_types.Client:
        """Fetches a client from the database, by a client_id

        Raises:
            ValueError: A client with the given client_id doesn't exist
        """

    @abstractmethod
    def get_client_list(
        self,
        chunk_size: int = config.DB_CHUNK_SIZE
    ) -> Iterator[List[db_types.Client]]:
        """Fetches all clients from the database

        Args:
            chunk_size: the iterator yields the clients in chunks,
                use this argument to control the maximum amount of
                client in each chunk.

        Returns:
            An iterator that yields clients from the database.
            Be aware that if a new client was registered while
            you were polling data, there is high possibility
            that you'll poll this client as well; you can not
            predict how many results this function will return.
        """

    @abstractmethod
    def update_last_seen(self, client: db_types.Client) -> None:
        """Updates the last seen field of a client to 'now'

        will not do anything in the case client can't be found
        """

    @abstractmethod
    def create_message(self, sender: db_types.Client,
                       receiver: db_types.Client,
                       message_type: pt_types.MessageType,
                       content: pt_types.MessageContent) -> pt_types.MessageID:
        """Adds a new message to the db

        A message in the database represents a pending message,
        that the sender sent to the receiver, but the receiver is yet to poll
        it from the database. Once the receiver polls the message from the server,
        the message is supposed to be deleted from the database.

        Since you provide both receiver and sender as structures,
        the function assumes they exist. Expect an undefined behavior otherwise.

        Returns:
            The id of the newly constructred message.
        """

    @abstractmethod
    def get_messages(
        self,
        receiver: db_types.Client,
        chunk_size: int = config.DB_CHUNK_SIZE
    ) -> Iterator[List[db_types.Message]]:
        """Fetches all pending message from the database

        Args:
            receiver: the receiver, the function assumes the receiver exists,
                yet nothing will happen if the receiver doesn't exist.
            chunk_size: the iterator yields the messages in chunks,
                use this argument to control the maximum amount of
                messages in each chunk.

        Returns:
            An iterator that yields messages from the database.
            Be aware that if a new message was sent while
            you were polling data, there is high possibility
            that you'll poll this client as well; you can not
            predict how many results this function will return.

        Notes:
            It doesn't delete the retrived messages from the database,
            please use delete_messages, after successfully calling for this function.
        """

    @abstractmethod
    def delete_messages(self,
                        message_ids: Iterator[pt_types.MessageID]) -> None:
        """Deletes a list of message ids from the database

        It's ok if the list contains ids of messages that were not in the database
        to begin with.
        """
