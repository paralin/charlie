Security: Critical
========

Boost is compiling some source code paths into strings... Possibly
redefine `__FILE__`


Security: Noncritical
=====================

Find a way to completely disable rtti in boost. typeid() calls are
leaking the names of all of the boost classes.


Efficiency: Noncritical
==================

Currently BOOST is compiling a lot of exception strings that don't need
to be there, like "boost unique_lock has no mutex" or "day of month
value is out of range"

Planning: Critical
==================

How to handle different operating systems?

Best solution is probably to make a set of modules for each operating
system supported. Make an OS parameter on the module object.

Security: Critical
=================

Need to verify base64 is valid everywhere it is used. At the moment
fudged base64 could be fed into the system 
