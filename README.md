Implementation of a system for online banking. Within the system
is considered to be the existence of two types of entities: a banking server offering two services: banking (for
verifying bank account, numeric transfer, ...) and unlocking service; and the client, which will allow users to access
the facilities offered by the server.
At start up, the server receives two arguments: a port number and the name of a file that holds the data
clients, whose structure will be explained later (Server Data Section). The way of
server startup is:
./server <port_server> <users_data_file>
A client will receive as arguments when starting the IP and server port. The client's startup mode is:
./client <IP_server> <port_server>
The server and client implementation code will be sequential (no more threads will be used) and the selective call for multiplexing the communication (with system entities and / or interface
user).
