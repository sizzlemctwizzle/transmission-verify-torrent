#!/bin/bash

# This script automatically verifies a torrent after
# it completes. All the heavy lifting is done by 
# "transmission-verify-torrent", which you need to build
# and install (make && sudo make install). Ignore the
# stupid build warnings.

# Uncomment for debugging
# TR_TORRENT_ID=$1

# Put the following code at the very top
# of your transmission completion script.

# Verify torrent data upon completion
status=`transmission-verify-torrent $TR_TORRENT_ID`

# Check if the torrent really is complete
if [ $status != "Complete" ]
then
    echo "Torrent is not complete."
    echo "Status is $status."
    exit
fi

echo "Torrent is $status."

# Put the rest (if any) of your completion script after this
