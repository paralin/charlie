# CNC
Central server, running through a SOCKS5 proxy or not, communicates through a bare socket with protobuf
The public key is encoded into the binary where might add additional encryption down the line.

Message header is message ID, message length, and signed hash of the message itself.

In this way every single command coming from the master server is signed and confirmed.

# Modules
Every single part of the program runs in modules.

Every module can be swapped out at runtime without restarting the **core**

It is expected that the **core** will remain the same always.

Furthermore all of the components have a version #. Every single component is signed and verified before being loaded.

Core list of modules is
    - Comms - Direct
    - Comms - TOR
    - Info (can gather system info)

# Module Table
Core data table (signed, especially when stored).

When the system first connects to CNC it requests an update to this table.

Updates can also be pushed out at will from the CNC

When the core detects an update to a component it requests a download for it, updates, etc.
