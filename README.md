# peer to peer file sharing software 
## This P2P protocol will use the centralized index approach. Using two programs,
 a centralized 'p2pregistry' and a client/server application 'p2pservent'. The protocol implements the four key
 operations of P2P applications: join, publish, search, and fetch.
### Join: 
The registry will listen to a TCP socket at a well-known port. A new servent will contact the registry
at this port and indicate its presence. In addition, the registry also listens to a UDP socket and every 60
seconds the servent issues a 'HELLO' message to the UDP port of the registry. If the registry has not
received a HELLO message from a servent in 200 seconds, the servent is removed from the list of available
servents.
### Publish: 
After registration, the servent sends a list of files available at the servent to the registry (e.g., all
files in a directory specified as command-line parameter). 
### Search: 
A servent looking for a file contacts the registry. The registry looks for the file in its database
(matching names) and returns the contact address of the servent if an entry was found (the first matching
entry).
### Fetch: 
The searching servent contacts the servent with the file and requests the transfer of the file across
a reliable socket and places the file in the same directory as the shared files.
