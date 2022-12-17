"""Handles new connections to the server

Every connection is handled as a thread,
and work concurrently to other connection.

Example:
conn, addr = sock.accept()
Connection(db_instance, conn).start()
"""
from __future__ import annotations
from typing import Union

from io import BytesIO
import threading
import socket
import logging
import tempfile

from protocol import request
from protocol import response
from protocol import types as pt_types
from protocol import exceptions as pt_exceptions
from database import engine_interface as db_engine
from database import types as db_types
import utils
import config

logger = logging.getLogger(__name__)


class Connection(threading.Thread):
    """A new connection to the server"""
    def __init__(self, database: db_engine.Database, conn: socket):
        threading.Thread.__init__(self)
        self._db = database
        self._sock = utils.Socket(conn)

    def run(self):
        try:
            server_response = self._process_request()
            for data_chunk in server_response.write():
                self._sock.send(data_chunk)
        except Exception as err:  # pylint: disable=broad-except
            logger.debug('Could not complete a request, reason: %s', err)
            return

    def _process_request(self) -> response.ResponseSchema:
        """High-level processing of the request"""
        header = request.Header.read(self._sock)
        handlers = {
            request.Register.CODE: self._register_request,
            request.ClientList.CODE: self._retreive_client_list,
            request.PublicKey.CODE: self._get_public_key,
            request.SendMessage.CODE: self._send_message,
            request.PendingMessages.CODE: self._retreive_pending_messages,
        }
        handler = handlers.get(
            header.code.value,
            lambda _: response.Error(),  #default behavior
        )
        return handler(header)

    def _register_request(self, header: request.Header):
        data = request.Register.read(
            BytesIO(self._sock.recv(header.payload_size.value, True)))
        if b'\0' not in data.username.value:
            return response.Error()  # the name must be null terminated
        try:
            client_id = self._db.create_client(data.username, data.public_key)
            return response.Register(client_id)
        except ValueError as err:
            logger.debug('Could not create a new client, reason: %s', err)
            return response.Error()

    def _retreive_client_list(self, header: request.Header):
        requester = self._login(header.client_id)
        if not requester:
            return response.Error()
        # We need to predict the payload size before sending it
        dump_payload = tempfile.TemporaryFile()
        for client_chunk in self._db.get_client_list():
            for db_client in client_chunk:
                if db_client.client_id.value == requester.client_id.value:
                    continue  # avoid returning the requester
                client = response.ClientListNode(
                    db_client.client_id,
                    db_client.username,
                )
                if (client.SIZE + dump_payload.tell()
                    ) >= pt_types.PayloadSize.MAX_PAYLOAD_SIZE:
                    break  # we can not add more clients to the response
                for data_chunk in client.write():
                    dump_payload.write(data_chunk)
        payload_size = dump_payload.tell()
        dump_payload.seek(0)

        return response.ClientList(
            (dump_payload.read(chunk_size) for chunk_size in
             utils.get_chunk_sizes(payload_size, config.DATA_CHUNK_SIZE)),
            payload_size,
        )

    def _get_public_key(self, header: request.Header):
        requester = self._login(header.client_id)
        if not requester:
            return response.Error()
        data = request.PublicKey.read(
            BytesIO(self._sock.recv(header.payload_size.value, True)))
        try:
            target = self._db.fetch_client(data.client_id)
            return response.PublicKey(target.client_id, target.public_key)
        except ValueError:
            logger.debug(
                '%s tried to get the public_key of an unregistered client(%s)',
                request, data.client_id)
            return response.Error

    def _send_message(self, header: request.Header):
        sender = self._login(header.client_id)
        if not sender:
            return response.Error()
        try:
            data = request.SendMessage.read(self._sock, header.payload_size)
        except pt_exceptions.ProtocolError as err:
            logger.debug('%s', err)
            return response.Error()
        try:
            receiver = self._db.fetch_client(data.receiver_id)
        except ValueError as err:
            logger.debug(
                '%s tried to send a message to an unregistered client(%s)',
                sender.client_id, data.receiver_id)
            return response.Error()
        try:
            message_id = self._db.create_message(
                sender,
                receiver,
                data.message_type,
                data.message_content,
            )
        except OverflowError as err:
            # Even though the protocol allows files of size 4GB~,
            # SQLITE has its own limits.
            logger.debug(
                '%s tried to send a message of size %s but received an error(%s)',
                sender.client_id, data.message_content.size, err)
            return response.Error()

        return response.MessageSent(receiver.client_id, message_id)

    def _retreive_pending_messages(self, header: request.Header):
        receiver = self._login(header.client_id)
        if not receiver:
            return response.Error()
        # It's possible that we'll have to retrieve thousands of
        # messages. However, we do limit the size of the payload we return,
        # so we shouldn't have a problem using a temp file, and we don't
        # have to worry about the extensive reading & writing.
        dump_payload = tempfile.TemporaryFile()
        dump_message_ids = tempfile.TemporaryFile()
        for message_chunk in self._db.get_messages(receiver):
            for db_message in message_chunk:
                message = response.Message(
                    db_message.from_client.client_id,
                    db_message.id,
                    db_message.type,
                    db_message.content,
                )
                if (message.get_size() + dump_payload.tell()
                    ) >= pt_types.PayloadSize.MAX_PAYLOAD_SIZE:
                    # If the message is too big, we'll have to skip it this time
                    # the receiver still have a chance to fetch it the next time.
                    continue
                for data_chunk in message.write():
                    dump_payload.write(data_chunk)
                dump_message_ids.write(db_message.id.write())
        payload_size = dump_payload.tell()
        dump_payload.seek(0)

        # Delete the messages we decided to return this time
        def generate_ids(dump_data):
            dump_data.seek(0)
            while True:
                data = dump_data.read(pt_types.MessageID.SIZE)
                if not data:
                    break
                yield pt_types.MessageID.read(BytesIO(data))

        self._db.delete_messages(generate_ids(dump_message_ids))

        return response.PendingMessages(
            (dump_payload.read(chunk_size) for chunk_size in
             utils.get_chunk_sizes(payload_size, config.DATA_CHUNK_SIZE)),
            payload_size,
        )

    def _login(self,
               client_id: pt_types.ClientID) -> Union[db_types.Client, None]:
        """Handles the login process

        Returns:
            The client object if login succeeded, otherwise None.
        """
        try:
            client = self._db.fetch_client(client_id)
            self._db.update_last_seen(client)
            return client
        except ValueError:
            logger.debug('Blocked unauthorized request for %s', client_id)
            return None
