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

# TIME SYNC ERRORS

If the server responds saying the timestamp is too old or invalid, the
client should try to maintain its own internal clock from internet
sources for the time and re-attempt connection. If impossible to fix the
problem the client will enter a hibernation state in which it will wait
for another re-attempt command from the server. This is a rescue state
where perhaps if numerous clients have time sync problems I can allow
the server to be more lax with timestamps in the future.

# POTENTIAL VULNS

Need some way to verify the initial command module before loading it.
This is hard because it might change.

# SAVE FILE

Identity and various other things stored in a save file next to the
core. The filename is based on the machine fingerprint.

The filename is XOR shifted by the system id.

# CORE MODULE SEQUENCE

The core module has no functionality that can be swapped out at a later
time (without some gymnatics). Sure, there could be an update module
that kills the process and launches an update process, but this would be
something developed down the line.

  1. Load saved data (could have a limited version of the proto message,
     this should be OK)
  2. Generate missing required data (like initial module table)
  3. If the management module doesn't exist drop it from the embedded
     library
  4. Load the management module and init it

# MANAGEMENT MODULE SEQUENCE

Here is the order of steps taken by the management module

  1. Load saved data (if any)
  2. Generate missing data (including identity information)
  3. Connect to server
  4. Send a CMsgClientRegister with identity and machine info object
  5. Handle any errors returned by server
  6. Send a CMsgRequestModuleTable
