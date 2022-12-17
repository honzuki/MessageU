"""The main module of the MessageU server

"""

import socket
import logging

import config
from database import sqlite3_engine
import connection


def main():
    """Entrance point

    The function takes care of initializing all needed structures,
    and accepting incoming requests.

    In the case of an exception, the function will log the event
    and exit safely.
    """
    try:
        config.setup_logger_config()
        port = config.load_port()
    except FileNotFoundError as err:
        print('The server can not start, you are missing config files....')
        return

    try:
        database = sqlite3_engine.Sqlite3Engine(config.DATABASE_NAME)
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.bind(('', port))
            sock.listen()
            logging.info('Server starts listening on port %i', port)
            while True:
                conn, addr = sock.accept()
                logging.info('New connection from %s', addr)
                connection.Connection(database, conn).start()  # auto detaching
    except Exception as err:  # pylint: disable=broad-except
        logging.exception('The server was terminated with error: %s', err)
        return


if __name__ == "__main__":
    main()
