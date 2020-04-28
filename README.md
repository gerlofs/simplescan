## Information:
This is a very basic "Full Connect" port scanner developed as a practice application, as highlighted in [TJ O'Connor's Violent Python](https://www.amazon.co.uk/Violent-Python-Cookbook-Penetration-Engineers/dp/1597499579) and [Jon Erickson's "Hacking: The Art of Exploitation"](https://www.amazon.co.uk/Hacking-Art-Exploitation-Jon-Erickson-ebook/dp/B004OEJN3I). Traditionally sockets are much more difficult to implement and use in C, allowing for a lot of reading and understanding of how networked applications work. 

## Setup:
* Compile using  `./compile.sh` on UNIX systems.

## Running:
* Use the `-a` cl argument to add a hostname (e.g. `www.google.com`) - _note:_ invalid hostnames are allowable (e.g. `http://www.google.com/`). 
* Use the `-p` cl argument to add a list of ports you wish to connect to/scan, if no argument is given a list of 20 useful ports are selected.

## TODO:
* Need to add a cl argument to control the timeout (in seconds) for the socket connection.
* Add additional scan types (e.g. half-connect, FIN, etc).
