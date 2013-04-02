#!/bin/bash

# This script automatically verifies a torrent after
# it completes. All the heavy lifting is done by 
# "transmission-verify-torrent", which you need to build
# and install (make && sudo make install).

# Uncomment for debugging
# TR_TORRENT_ID=$1

# Put the following code at the very top
# of your transmission completion script.

# Verify torrent data upon completion
# Usage: transmission-verify-torrent [host] torrent-id [options]
# Options:
#   -n   --auth       <user:pw>   Set username and password
#   -ne  --authenv                Set authentication info from the
#                                 TR_AUTH environment variable
status=`transmission-verify-torrent $TR_TORRENT_ID 2> verify-errors.out`

# Check if the torrent really is complete
if [ "$status" != "Complete" ]
then
    if [ "$status" != "" ]
    then
        echo "Torrent is not complete."
        echo "Status is $status."
    else
        echo "transmission-verify-torrent encountered errors:"
        cat verify-errors.out
    fi
    exit
fi

echo "Torrent is $status."

# Put the rest (if any) of your completion script after this
