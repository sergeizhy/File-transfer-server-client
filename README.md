server for the project 
- used  pycrypto pycryptodome
        cryptopp X:\xxxxx\includes\cryptopp
        boost X:\xxxxx\includes\boost

- the server and the client are using a ping pong protocol one throws a 
request the other replies from a pre defined hash table of responses

- both the server and client are aimed to be used on little endian machines

- the server uses ram sqlite called :memory: as a primary database to store data about the client
per each request the server copies pages from :memory: to the secondry database after crashing 
or manully closing and restarting the server data is loaded back again from secondry database 
to the prinmary database

- clients code is self explanatory define packet -> load the data to the relevent request -> split the packet
     -> encrypt -> split to the right buffer size -> send
same proccess is applied to receiving

- server code uses socketserver as a wrapper to handle data transaction for each chunk of data, data is processed
by StructHandler based on a ready memory templates 
data unpacking - the data for each request is unique which requires unique object the relevent object is created 
with the relevent attributes for the request to be translated 




