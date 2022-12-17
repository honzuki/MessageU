"""Handles config arguments

Used for user defined arguments, not constants defined
by the MessageU protocol.

"""

import logging
import time
import os

PORT_LOCATION = 'myport.info'
DATABASE_NAME = 'server.db'
LOGS_RELATIVE_PATH = 'logs'

# The size the maximum chunk size we use to [read from / write to] the network
DATA_CHUNK_SIZE = 1024

# The amount of rows in each chunk of results of a database query
DB_CHUNK_SIZE = 10


def load_port(location=PORT_LOCATION):
    """Loads a port number from a config file

    Args:
        location: the relative location of the config file.
    """
    try:
        with open(location) as file:
            port = int(file.read())
    except ValueError as err:
        raise ValueError('can not parse the port number from %s' %
                         location) from err
    return port


def setup_logger_config():
    """Sets the basic config of the logger"""
    os.makedirs(LOGS_RELATIVE_PATH, exist_ok=True)
    logging.basicConfig(
        filename='%s/%f.log' % (LOGS_RELATIVE_PATH, time.time()),
        format='[%(levelname)s] %(asctime)s %(message)s',
        datefmt='%Y/%m/%d %I:%M:%S %p:',
        level=logging.DEBUG,
    )
