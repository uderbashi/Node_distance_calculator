# Node distance calculator
This repo  has code for a client and a multithreaded server. The server starts with a graph, and the client requests the distance between two nodes.

The graph is taken from https://snap.stanford.edu/data/ , and then parsed and stored on the server side. It also spins multiple threads which recive connections from multiple clients to serve them.

The client just simply requests a distance betwen two nodes. The server returns it. The server nust not recalcuate paths it has already parsed.

All the data structures were implemented from scratch, and all the connections must have been done without, and I quote "sleep, goto, non blocking operations, timed waits or busy waiting of any kind".

This was submitted as an assignemnt for the Systems Programming course.
