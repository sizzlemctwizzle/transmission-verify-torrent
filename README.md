## Rationale

Sometimes live verification of data in Transmission doesn't work and you end up with a download that has completed but has bad blocks. The developers of Transmission are [unwilling](https://trac.transmissionbt.com/ticket/4649) to add an option to automatically verify the entire torrent upon completion, because they believe they can detect and fix any bad blocks while the download is in progress. If you know about the complexities of networking, hardware, filesystems, and I/O then you will understand 100% success is impossible. Edge cases can and do happen. It happened enough for me to take the time to develop a fix for myself.

The developers don't want to implement the option for full verification upon completion because users who encountered the problem the first time would likely turn on the feature, and that would mask the problems with live verification. Bugs would go unreported (although they could just send automated bug reports when this happens), and improvement of live verification would stop. Additionally, verifying a large download all at once can take a long time and will almost always be unneccessary. But if you prefer certainty over speed, and have no interest in reporting bugs with the live verification process then `transmission-verify-torrent` is the solution.

The simplist work-around is to use this completion script:

    #!/bin/bash

    transmission-remote -t $TR_TORRENT_ID --verify

Unfortunately, this means you will have to sacrifice running any other completion routine since this code just initiates the verification routine and doesn't return any information about whether the torrent data is valid.

Instead, **transmission-verify-torrent** initiates the verification, waits until it finishes, and then prints "Complete" to standard out if the torrent is actually complete or the torrent's current status if verification failed. See the **verify-on-completion.sh** script for an example on how to use it in your torrent completion script.

## Could you do all of this with a shell script?

Sure, that would save you the trouble of compiling. But I find that shell scripts are usually sloppy, whereas C is realiable and precise. Plus novice users are more likely to fool around and break a shell script but are unlikely to touch C code. They might think they can copy what they need and shove in other snippets to do other things. Then suddenly something breaks and they don't know why or how to fix it. I think it is a good idea to encapsulate the neccessary (missing from transmission) functionality in a compiled application and know you can count on it.

## Installing

To build and install **transmission-verify-torrent**, download the source, cd into the directory and run:

    make && sudo make install

`sudo make install` copies the binary into `/usr/local/bin`. If this isn't in your PATH then edit the Makefile and set `INSTLOC` to some path that is. It depends on your OS and system configuration, so figure it out. Or use a relative path to this binary in you completion script.
