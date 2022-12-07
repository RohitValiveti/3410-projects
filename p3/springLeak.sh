#!/bin/bash


INPUT="%x %x %x %x %s"

# This line of code will send the contents of the "INPUT" variable
# to the listening server and then will capture the response for the server in the variable TMP.

TMP=$(echo -n "$INPUT" | nc -u 127.0.0.1 8080 -w1 | cut -d ' ' -f 5)

# TODO craft a special INPUT that will leak the secret and then
# modify this script to output that secret

# YOU WILL LIKELY HAVE TO modify the following to get it to JUST print the SECRET
echo "$TMP"
