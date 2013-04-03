## Rationale

Sometimes live verification of data in Transmission doesn't work and you end up with a download that has completed but has bad blocks. The developers of Transmission are [unwilling](https://trac.transmissionbt.com/ticket/4649) to add an option to automatically verify the entire torrent upon completion.

The simplist work-around is to use this completion script:

    #!/bin/bash

    transmission-remote -t $TR_TORRENT_ID --verify

Unfortunately, this means you will have to sacrifice running any other completion routine since this code just initiates the verification routine and doesn't return any information about whether the torrent data is valid.

Instead, **transmission-verify-torrent** initiates the verification, waits until it finishes, and then prints "Complete" to standard out if the torrent is actually complete or the torrent's current status if verification failed. See the **verify-on-completion.sh** script for an example on how to use it in your torrent completion script.

## Installing

To build and install **transmission-verify-torrent**, download the source, cd into the directory and run:

    make && sudo make install

It seems the very latest version of Transmission can list info about a single torrent using:

    transmission-remote -t $TR_TORRENT_ID -l

But because this change is still very new, it's not utilized by default in **transmission-verify-torrent**. Instead it searches through the entire list of torrents for the desired torrent. However, if you've verified that the above command lists only the single torrent then you can build to utilize this ability using:

    make SINGLE_TORRENT=1 && sudo make install

