#!/bin/bash
PORT=12345  # Change this to the desired port

while true; do
    # Listen on the specified port and fork a new background process for each connection
    nc -l -p $PORT -k &
done
