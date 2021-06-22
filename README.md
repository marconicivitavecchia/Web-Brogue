Web Brogue
==========

A web[sockets] server for playing the Brogue over the internet.  Brogue is a game for Mac OS X, Windows, and Linux by Brian Walker.  For more information go https://sites.google.com/site/broguegame/.  The server only can be run on a POSIX environment at the moment.

Build Instructions
-----------------------

### Step 1: Get Dependencies ###
Get the latest stable version of node.js and mongoDB. (Or whatever mongoDB comes with your distro which should be fine.)

### Step 2: Get node packages
Navigate to the `server` directory and run `npm install` to get the node dependencies. You'll need a C++ compiler to compile the datagrams module.

### Step 3: Build Brogue Executables ##
The paths to the binaries is stored in `server/config.js`.
Brogue binaries are either compiled and referenced in place, or stored in the `binaries/` directory. Best practice is to place binaries in the `binaries/` directory. If a replay-breaking change is added to a brogue binary, create a new variant and add a new brogue binary. This allows old recordings to still work. (Note that a new variant means separate high scores for now.)

* Brogue (1.7.4): `cd brogue` `make web`
* Brogue (1.7.5): `cd brogue-1.7.5` `make web`
* gBrogue: `cd gbrogue` `make -f Makefile.linux web`
* unBrogue: `cd unBrogue` `make web`
* broguePlus: `cd brogueplus` `make web`
* Brogue CE (1.8): Clone `https://github.com/flend/BrogueCE` and checkout branch `tracking/web-brogue`. `make bin/brogue` then copy binary to `binaries/brogue-ce18`
* Brogue CE (1.9): Clone `https://github.com/flend/BrogueCE` and checkout branch `tracking/web-brogue-v193`. `make bin/brogue` then copy binary to `binaries/brogue-ce19-c813284` (or similar filename)
* Brogue CE (1.10): Clone `https://github.com/flend/BrogueCE` and checkout branch `tracking/web-brogue-v110`. `make bin/brogue` then copy binary to `binaries/brogue-ce110` (or similar filename)
* Rapid Brogue: Clone `https://github.com/flend/BrogueCE` and checkout branch `tracking/rapid-brogue-v100`. `make bin/brogue` then copy binary to `binaries/rapid-brogue-v100` (or similar filename)

Starting the Server
----------------------------

Starting the server should be as simple as starting up mongoDB and starting the node process.

1. To start the mongodb daemon type `mongod` (this should be started at system init in most cases)
2. To start the server type `npm start` in the `server` directory.

You will probably want to edit `server/config.js` and `client/config.js` to set the ports and the server secret.

If everything is running correctly it should say "Server listening on port XXXX"

Note that you might need root access if you are running on a privileged port (e.g. 80).

Configuration
--------------------------------
Server global configuration variables are defined in `server/config.js`.
Client configuration variables are defined in `client/config.js`.

In particular, make sure to get the client and server to agree on the websocketPort.

Tests
----------------------------
An API test suite can be run using `npm test` in the `server` directory.