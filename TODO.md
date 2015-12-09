 - Add concept of "master keypairs" or trusted "commanders" - people
   issuing commands.
 - "server keypairs" or "trusted relay keypairs" are the C&C server keys
   that the client will trust. The idea here is to have one keypair for
   every trusted server, but not to actually trust the servers
   themselves for commands, but the person sending the commands.
 - Build a framework for hiding file templates better (i.e. autostart
   file)
 - Randomly generate the XOR keys for everything to make pattern
   generation harder.
 - Implement a compiler in the server to randomize some things within
   every single build of the executable in order to make things more
   confusing
 - Add the llvm obfuscation work in for modules

Possible bugs:

 - When the module table is reloaded all of the CModules are probbly
   released.
 - The module instances use CModule pointers to hold state. Make sure
   these CModule state elements are replicated to the new module table
   and the pointers are updated.
