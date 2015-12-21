# CNC
Central server, running through a SOCKS5 proxy or not, communicates through a bare socket with protobuf

The public key is encoded into the binary where might add additional encryption down the line.

**NOTE**: Since the beginning of this project I've decided to use what
is currently called the "server identity" as just the key used to sign the
online module tables. These can come through anywhere. "server_keys"
are the public keys trusted as command servers.

Message header is message ID, message length, and signed hash of the message itself.

In this way every single command coming from the master server is signed and confirmed.

# Modules
Every single part of the program runs in modules.

Every module can be swapped out at runtime without restarting the **core**

It is expected that the **core** will remain the same always.

Furthermore all of the components have a version #. Every single component is signed and verified before being loaded.

Core list of modules is
    - Manager
    - Persist
    - Client
    - Tor proxy

# Server Modules

Every module has a server component. The server component is responsible
for communicating with the client component and handling messages /
managing things.

There is a 1<->1 relationship between server and client modules. Thus,
when a client handshake completes, the server will construct *all* of
the modules on the server for the client.

Server modules should be lightweight.

# Client Identification

Storing the clients in the database is a bit of an interesting problem.
We never trust anything the client sends to the server to be true. It
could be someone trying to reverse engineer the protocol, and mess
everything up.

Spamming tons of new clients to overflow the db with fake records is
fixed using Proof of Work for registration which will be covered later
on.

In the meantime, the system fingerprint is a good way to identify a
client. But the problem is, this could be arbitrarily set / faked by a
fake client. So it can't be used as the `_id` field in the database.

The solution to this is to simply use the md5 of the public key string
as the identifier. Store the fingerprint, but don't trust it.

# Module Table
Core data table (signed, especially when stored).

When the system first connects to CNC it requests an update to this table.

Updates can also be pushed out at will from the CNC

When the core detects an update to a component it requests a download for it, updates, etc.

# TIME SYNC ERRORS

If the server responds saying the timestamp is too old or invalid, the
client should try to maintain its own internal clock from internet
sources for the time and re-attempt connection. If impossible to fix the
problem the client will enter a hibernation state in which it will wait
for another re-attempt command from the server. This is a rescue state
where perhaps if numerous clients have time sync problems I can allow
the server to be more lax with timestamps in the future.

# SAVE FILE

Identity and various other things stored in a save file next to the
core. The filename is based on the machine fingerprint.

The filename is XOR shifted by the system id.

# INITIAL MANAGEMENT MODULE

At least one management module is required at all times to bootstrap the
system. The executable comes with the binary of the initial management
module built in along with a matching initial module table.

# MODULE STRUCTURE

To the core, every module has to be EXACTLY. the same.

When a module is loaded the system searches for and instantiates the
`Module` feeding in the manager.

# CORE MODULE SEQUENCE

The core module has no functionality that can be swapped out at a later
time (without some gymnatics). Sure, there could be an update module
that kills the process and launches an update process, but this would be
something developed down the line.

  1. Load saved data (could have a limited version of the proto message,
     this should be OK)
  2. Generate missing required data (like initial module table, identity
     information)
  3. If the management module doesn't exist drop it from the embedded
     library
  4. Load the management module and init it

# MANAGEMENT MODULE SEQUENCE

Here is the order of steps taken by the management module

  1. Load saved data (if any)
  2. Load the list of network comms plugins (if none found, proceed to
     INIT SEQUENCE)
  3. Iterate over the plugins by priority figure until contact is
     successful (if contact is not successful on any attempt REINIT
SEQUENCE)
  4. Send a CMsgClientRegister with identity and machine info object
  5. Handle any errors returned by server, validate the response from
     the server to make sure it's a valid remote server.
  6. Send a CMsgRequestModuleTable

# SERVER PORTS
INTERNAL PORT | TOR PORT


# INIT SEQUENCE
If you go to `/initt` you get the initial module table base64 encoded.
This data is also uploaded to hastebin and a few other places and the
tiny.cc urls updated to point to the latest table. The client will check
all of the locations and use the newest module table it finds. Note the
type of the encrypted data is CSignedBuffer so it's impossible to fake
out the client into downloading wrong modules.

The client gets the latest module table and goes through the DOWNLOAD
MODULES sequence. After that sequence is complete it tells the main
client manager to reload the manager module (which will pull the latest
updated manager). Then the system should start again and see the network
modules and use those to communicate with the server.

# REINIT SEQUENCE
This happens when the client can't connect on any front. It will attempt
to fetch the latest init module table from the INIT SEQUENCE. If there's
no update to the table it won't restart but just continue to attempt to
connect normally. But, this will set a timer to re-check the init table
every half an hour if contact is not re-established.

# Handshake Sequence V2

This is the sequence for connecting to a server:

  1. Open socket connection.
  2. Server sends a CServerIdentify. Contains the server public key.
     Contains the challenge for the client.
-- start RSA encryption --
  3. Client sends a CClientIdentify. Contains client pubkey. Contains
     challenge for server.
  4. Server sends a CServerAccept with verification signature for
     challenge.
  5. see reg sequence

# Registration Sequence

If the server doesn't recognize a client (unknown client) it sends a
CServerRegisterChallenge with a random value to tag with a proof of work
function, and the required bit count to be correct in the POW function.
It then terminates the connection.

# Tor Module
Two modules. One that runs tor on a localhost port, and another that
actually is the client to the server.
