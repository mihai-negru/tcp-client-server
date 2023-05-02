# Negru Mihai 323CD: PCom Assignment No. 2


## `Getting Started`


This `README` tries to explain how the assignment was done.
* Mainly here we will discuss the implementations and the motivation for choosing one idea over another.
* For more information you can lookup in the code for comments that explain details.
* The details as *"I set that fields to that value because of...."* can be found in the code at the following files:
    * [utils.c](./src/utils.c) - Basic utils file for handling errors (defined a error enum class).
    * [udp_type.c](./src/udp_type.c) - Parsers and structures to handle an UDP message from an UDP Client.
    * [tcp_type.c](./src/tcp_type.c) - Definition for the protocol over the tcp protocol.
    * [subscriber.c](./src/subscriber.c) - Main thread for a subscriber.
    * [subscriber_utils.c](./src/subscriber_utils.c) - Utils for handling server messages (part of the protocol).
    * [server.c](./src/server.c) - Main thread for a server.
    * [server_utils.c](./src/server_utils.c) - Utils for handling server and clients messages (part of the protocol).
    * [poll_vec.c](./src/poll_vec.c) - Simple vector definition for IO multiplexing.
    * [client_vec.c](./src/client_vec.c) - Simple clients vector storing clients metadata.

In the following sections we will go through the following ideas:
* Handling errors with "*beautiful*" methods.
* General functionality of the server/subscriber.
    * Handling **stdin** input messages.
* Handling IO multiplexing.
* Handling messages from an UDP Client.
* Handling messages from a TCP Client:
    * The basic idea for the protocol.
    * Implementing the protocol.
    * Describing the client structure.
    * Handling protocol messages by server/subscriber.


## `Error handling`


### `Enum typedef err_t`


This enum is the main "error class", every function from this project API returns an `err_t` enum, which contains significant information about any error that could happen.

The enum error class contains a `variant` called **OK**, which signals that the function returned with a normal behaviour.

>**NOTE:** We define the *normal behaviour* as the expected behaviour of a function.

The enum also contains an **OK_WITH_EXIT** variant, which singals that the function return also with a normal behaviour, however it requests the termination of the current processing thread, because of the messages that it has received or depending on other variables.

The error enum is a collection of all errors, both for server, subscriber, poll vector, client vector, udp parser, tcp parser.

>**NOTE:** The whole project respects the current error handling **protocol**by processing functions which return an `err_t` and they also return an `err_t`.


### `Isomorphism between err_t and socket's API errors`


The *err_t* enum respects the socket's API regarding handling errors and defines `variants` to match the serros coming from socket API, for example:

* 0 bytes were read from the tcp pipeline <=> TCP_FAILED_SEND_RECV
* accept function returned with error <=> SERVER_FAILED_ACCEPT_TCP
* and so on...

>**NOTE:** The socket API errors are not mappes one-to-one to `err_t` enum and two different socket errors can refer to a single err_t variant and vice versa.

>**NOTE:** If the name is just not enough, you can call `debug_msg` function on a `err_t`variable in order to find more informations about the current errors that was throwed.


## `Main functions of the server/subscriber`


### `Server`


The main functionalities of the server are:

* Reads commands from the standart input.
* Fetches and parse messages from the UDP Clients.
* Sends topics to subscribed TCP Clients.
* Subscribes/Unsubscribes TCP Clients to topics.
* Maintains a store-and-forward functionality.

If a fatal error is caught, for example the **listener** socket is closed the server will terminate its process and will terminate all processes for the active clients.

>**NOTE:** Upon closing the server just closes all the opened file descriptors, which generates a message to the other side and clients upon receiving a **0 bytes** message length understand that have to close themselves.

The server can also get commands from the input console, the only valid command is:

* **exit** - case sensitive

Other commands will just be ignored and will not crash the server process.


### `Subscriber`


The main functionalities of the subscriber are:

* Reads commands from the standart input.
* Sends subscribe/unsubscribe commands to the server.
* Fetches tcp messages from the server and processes the data.

>**NOTE:** By processing the message from the server I mean printing it, because it is the only thing the subscriber can do with it at the moment.

Upon connecting the subscriber with the server, if the connection is accepted by the server, the client must provide an **ID** (sending it via the defined protocol which we will discuss it later), the server gets the ID and checks if the client can continue with the connection with the server.

You can look at this as a **Three handshake** in TCP Protocol.

If the client can continue with the connection the server will not send any additional information, otherwise the server will close the client process, by closing its file descriptor which will generate a **0 bytes** read message.


#### `Subscriber commands`


The available commands that a client can receive from the standart input are:

* *subscribe topic sf* - `topic` is a topic name and sf(0/1) means to disable/active the sf option.
* *unsubscribe topic* - unsubscribes the client from a topic.
* *exit* - exits the current client process and signals the server.

The subscriber can `subscribe` or `unsubscribe` from a non-existent message, which is not considered an error. For example the following commands will generate the following output:

```bash
    ./subscriber id ip port
    subscribe to_a_topic_name 1
    Subscribed to topic. # Even if the topic does not exists

    unsubscribe to_a_topic_name
    Unsubscribed from topic.

    # From the server side
    ./server port
    New client id connected from port:ip.
    [DEBUG] Cound not find the topic in order to remove. # Not an error just an warning
```

The input command name is **case sensitive** and has to follow the current pattern, however for subscribing command the client can match the wrong input with a correct one, for example:

```bash
    ./client
    subscribe to_topic # <=> subscribe to_topic 0
    subscribe to_topic -1321 # <=> subscribe to_topic 0
    subscribe to_topic 43 # <=> subscribe to_topic 1
    subscribe 43 # in this case the topic name will be "43" <=> subscribe 43 0
    
    subscribe to a fragmented topic 0 # subscribe to 0
    subscribe to a fragmented topic 1 # subscribe to 0
    
    subscribe # Actual error because the topic name cannot be inferred
    [DEBUG] Input command has not a valid format.
```

The wrong format of the input will not generate closing of the client, but just printing an warning (all the debug messages are printed in the stderr and cannot inffer with the stdout).

>**NOTE:** If the input could ne matches with a valid input format the client will not print any debug messages and will procced to send the request to the server in order to get subscribed or unsubscribed


## `IO multiplexing`


The server and subscriber both use IO multiplexing in order not to stop to a blocking **recv** or **fgets** function call.

In order to use IO multiplexing we will use the **pollfd** structure with **poll** function call.

In order to perserve a structure and a *single principle responsability*, the client and server both have a **poll_vec** structure which will handle the inserting, deleting the file descriptors.


### `Client`


The client will have just two **fds** in the poll vector, the *stdin* and the *tcp socket* that is used to send and receive messages from the server.


### `Server`


The server will have an undefined number of **fds** at a momemnt of time. First the pol lvector will contian the stdin for fetching commands an *udp socket* for receiving messages from UDP Clients and a *listener socket* for connecting TCP Clients to the server, then for every accepted connection we will have a new socket file descriptor in order to communicate with the desired client.

Every time a client disconnects the server will remove its file descriptor from the poll vector to optimize the searching process.

If the **udp socket** or **listener socket** closes this will cause the server to terminate it's process by freeing its resources and closing the main thread of the process, because the server should not work (loses the idea of a broker) without any of these sockets.


## `UDP Client`


The format of a message from a UDP Client is defined as a:
    * *topic_name* - string of max 50 characters
    * *topic_type* - 1 byte
    * *topic_data* - dynamic size according to the type

The server waits for **POLLIN** on the udp socket and receives a message from a UDP Client, the messages is parsed into an internal structure, which encodes the the format specified above using `struct` and `union`, after that the message is send to the subscribed TCP Clients and is saved on the local server storage in case it was not sent to all the clients (speaking of disconnected clients).


## `TCP Client`


A TCP Client is a valid entity from the server, not just a message, so it has many more functionalities than just a simple UDP Client.

The TCP Client is an entity that stores metadata for a subscriber inside the server so it a *part of a greater good...*

The TCP Client structure contains the following important fields:

* *id* - the unique ID assign to a client.
* *status* - ACTIVE or DEAD.
* *fd* - the tcp socket from the server side in order to send and receive messages.
* *topics* - vector of string containing subscribed topic names.
* *ready_msgs* - **POINTERS** to udp messages that need to be send upon a stable connection between server and TCP Client.

When a subscriber sends a command to the server, the server gets its file descriptor and finds the client and modifies the metadata of the specified client in order to match with the subscriber preferences.


### `Protocol over TCP protocol`


We cannot just send data over the network, we should create a protocol over the TCP Protocol to handle every corner case.

Mainly the defined protocol is defined into three parts:

* *Sending/Receiving protocol*
* *Server-side protocol*
* *Subscriber-side protocol*

In the following sections we will discuss about all the parts that create the overall protocol.


### `Sending/Receiving protocol`


The sending and receiving is made using the following two functions:

```c
    err_t send_tcp_msg(const int tcp_socket, void *buf, size_t buf_len);
    err_t recv_tcp_msg(const int tcp_socket, void *buf, size_t buf_len);
```

Both them send/receive a **buffer** of the **buf_len** size, the function will not return until not **buf_len** bytes are send or received over the network, if at one step no bytes were send or received the function will return with a error that signal that sending or receiving over the tcp socket failed, even is some of the bytes were processed.

In order to encapsulate the buffer and to perserve a better structure of the protocol packages we define a new type for the protocol cargos:

```c
    typedef struct __attribute__((__packed__)) tcp_msg_s {
        uint16_t    len;
        char        data[MAX_TCP_MSG_BUF_LEN];
    } tcp_msg_t;
```

The server and the subscriber will use this structure in order to send a package over the network, the **len** fields specifies the number of meaningful bytes from the **data** buffer, so if one of the entities sends a `tcp_msg_t` it will receive also a `tcp_msg_t` structure.

>**NOTE:** Using the {send/recv}_tcp_msg we solve the problem with trimming or concatenation of the TCP STREAM.

>**NOTE:** Because the send_tcp_msg and recv_tcp_msg block until send or receive the desired number of bytes specified, which in our project will be always **sizeof (tcp_msg_t)**, we ensure that we will always get a valid message, because if the number of bytes were not full send received the packet is dropped and the operation should be taken one more time.


### `Server-side protocol`


The *server-side protocol* in other words is the expresed behaviour of the server program in different scenarious.

The *server-side protocol* can be found in the **process_ready_fds** in the **server_utils** file.

The protocol regarding tcp messages:

* The three handshake went successfully, the client was connected to server.
* The server receives the client's *ID*, if not something went wrong regarding the server so we will exit the server and will close the other client connections.
* The server tries to register the client into the local structures:
    * If registering fails it means that the client is already connected to the server.
    * If registering succedded it means that:
        * Client is a new client so allocate resources for it.
        * Client is reconnecting so update the metadata and set to ACTIVE.
* The server sends to the client all the topics with store-and-forward option set to true.
* The server moves next to handling other requests.
* The server receives a message from the client which is a command, because clients can send jsut commands of **subscribing/unsubscribing** from a topic.
* The server fetches the message, parses it and executes it using some internal functions for handling the commands.
* The server receives a message from an UDP CLient.
* The server parses the udp message, then parses it to the `tcp_msg_t` and send to all subscribed clients. If the client is subscribed with store-and-forward option the message will be stacked in a local storage regarding to the client and the server will attempt to send it over the next time a reconnection happens.
* If the client closes it's connection that the server will be notified and the protocol will set the client as DEAD and will remove the file descriptor from the poll vector.

This is a short revision for the *server-side protocol* on handling the **TCP MESSAGES**, more information can be found on the functions documentation.


### `Subscriber-side protocol`


The *subscriber-side protocol* in other words is the expresed behaviour of the subscriber program in different scenarious.

The *subscriber-side protocol* can be found in the **process_ready_fds**, **process_subscribe_cmd** and **process_unsubscribe_cmd** functions in the **subscribed_utils** file.

The protocol regarding tcp messages:

* The three handshake went successfully, the subscriber is connected to the server.
* The subscriber send in a **tcp_msg_t** format its ID.
* If the server accepted the ID it will NOT send anything to confirm it and will just keep the connection alive, if the ID is not ok, the server will close the connection (we should not to reinvent the wheel by sending a "close" message or "accepted Id" message it is done by the TCP protocol on the transportation protocol level).
* The subscriber waits for command line input in order to send over the network to server.
* The subscriber receives a command:
    * The subscriber tries to parse the input command.
        * The command can be formatted to a valid requet
        * The command cannot be formatted to a valid request so it is ignored.
* If the command was parsed successfully to a request, the **tcp_msg_t** with regarding data is send to the server, the message respects the correct format in order to be processed with easy by the server.
* The subscriber is waiting for topics, which will respect a single format...the message should represent a valid string, this format is respected by the server and server takes care to convert topic data into a single string and to send to the subscriber.
* If the subscriber gets a message from the server it will print to the stdout.
* If it read a **0 bytes** message the connection is closed so the client will shut itself.
* Sending input to requests to server and fetching topics data repeats until the connection is closed.

This is a short revision for the *subscriber-side protocol* on handling the **TCP MESSAGES**, more information can be found on the functions documentation.

For any details or clarifications be free to contact me to the following email address "determinant289@gmail.com"
