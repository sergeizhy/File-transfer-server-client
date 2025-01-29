# Defensive programing course

Used PyCrypto, PyCryptodome, and Crypto++ at X:\xxxxx\includes\cryptopp, as well as Boost at X:\xxxxx\includes\boost.

The server and client communicate using a “ping-pong” protocol: one side sends a request, and the other replies with a predefined response from a hash table.

Both the server and client are intended for use on little-endian machines.

The server uses an in-memory SQLite database (:memory:) as its primary data store to maintain client-related information. For each request, the server copies pages from the :memory: database to a secondary database. After a crash or after manually closing and restarting the server, the data is loaded again from the secondary database into the primary in-memory database.

The client’s code is self-explanatory: define a packet, load data into the relevant request structure, split the packet, encrypt it, split it into the correct buffer size, and send it. The same process applies when receiving data.

On the server side, we use socketserver as a wrapper to handle data transmission in chunks. The data is processed by StructHandler, which relies on predefined templates for data unpacking. Since each request is unique, it requires a unique object. The relevant object is created with the relevant attributes for the request to be translated.
