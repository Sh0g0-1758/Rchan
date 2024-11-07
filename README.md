# Rchan

## A p2p networking platform built on a custom TCP implementation.

The motivation for this project came from the lack of a private means of communication in universities. Most chat applications are centralized and provides little to no privacy. Rchan gives control over data to the user group who want to use it for their private communication as well provides a platform for all the university students to engage in a completely anonymous unhinged discussion about anything. 

The architecture of Rchan closely follows that of 4chan (its namesake) but with some changes. There is a central server (Rchan) which each user is connected to when he/she starts the client. After that the user has the option to enter any server(board). These can be Rchan (open to everyone) or any private server which are hosted by independant users. Rchan has no control over data in the latter case. To enter and host a server, users will need to enter a server password, with an additional root password when they host the server. If you are hosting your own instance of Rchan, make sure to make changes in the constructors of RchanClient and RchanServer, where the ip of Rchan server is hardcoded due to obvious reasons (Line 39 in apps/RchanServer.cc and Line 57 in apps/RchanClient.cc). 

## Build

To build Rchan run

```
./scripts/tun.sh start 144
mkdir build
cd build
cmake ..
make
```

## Run

To get the Rchan server / client running

```
./build/apps/server
```
```
./build/apps/client
```

## Test and Benchmarks

To test and benchmark our custom TCP implementation

```
make testRchan
```

```
make testSpeed
```

## Credits

This work was possible only because of the [networking lab at Stanford](https://cs144.stanford.edu). 

Made with ❤️ by: 

1. [Shogo](https://twitter.com/ShogLoFi)
2. [Kowalski](https://twitter.com/AnantJain99390)
3. [4m4n-x-b4w4ne](https://twitter.com/Abnwamsn)
4. [LoneWolf](https://twitter.com/atul_k_6)
