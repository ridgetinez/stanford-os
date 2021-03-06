### Bootloader prelab

For this lab, there is no official thing you need to hand in.  HOWEVER,
you are strongly encouraged to do all the prelab work, otherwise it's
going to be a long lab --- in particular, implement the two routines
described in Part 3 (below).

### 1. Read through the lab

You'll especially want to look through the protocol in the lab's Part
2 for context.  It's not that complicated, and everything else is only
interesting to the extent it enables you to build the protocol.

### 2. Look around the code.

To save time in lab it's good to get sort-of familiar with the starter
code and project code structure.  Since we are writing code to run on both
UNIX (`unix-side`) and on the r/pi (`pi-side`) we have more directories
than usual for today.  These are described below.

##### `pi-side` directory

The `pi-side` directory holds all the r/pi code.   In it, the
`pi-side/libpi.small` directory holds a stripped down `libpi`
implementation called by your pi-bootloader code.  The next few labs
will replace the `.o` files with your own implementation.

Make sure you look through:

  1. `pi-side/get-code.h` this holds the starter code that you will extend out
       into a full implementation that receives code from the UNIX side.  

  2. `pi-side/bootloader.c` has the driver that calls `get-code.h`.

  3. `libpi.small/rpi.h` contains the prototypes and descriptions of pi routines
      you (or our starter code) can call.  Each has a comment describing them:
      please post to the newsgroup if these (or any other function descriptions)
      are confusing!   These include routines to read and write memory (do not
      do it directly!), reboot the pi, and send/receive bytes from the UART
      (the hardware on the pi that talks to your TTY-serial device).

It's worth looking at the other files to.  In particular, compare the `staff-start.s` 
here to the `start.s` in your `1-gpio` lab:
   1. What do they even do?
   2. Why are they different?

##### `unix-side` directory

The unix side code is split into two directories:
   1. The `2-bootloader/unix-side` directory holds the UNIX side
      bootloader code.  
   2. `cs140e-21win-dev/libunix` directory holds useful UNIX helper
      routines that your unix-side bootloader code will use (as
      well as subsequent labs).  These routines are all described in
      `libunix/libunix.h`.

Make sure you look through:
  1. `simple-boot.c`: this holds the starter code you will extend
     out into a full implementation that sends code to the pi.
  2. `libunix/read-file.c`: you will implement this (see below).
  3. `libunix/find-ttyusb.c`: you will implement this (see below).
  4. `libunix/libunix.h`: this has all the routines you can use.
  5. `libunix/demand.h`: this has a variety of helpful macros to simplify
     error handling / checking.  You can see examples by using `grep` in
     the `libunix` directory.

And, look but DO NOT MODIFY:
  1. `libunix/put-get.c`: this holds helper routines to send/receive data
     to/from the pi.   Use these directly!
  2. `unix-side/my-install.c`: the driver for the unix-side bootloader.

### 3. Implement two Unix support routines.

To get you used to dealing with `man` pages and their ilk, you'll
write four routines needed by your bootloader.  Their prototypes are in
`libunix/libunix.h` and two simple C files that can serve as starter
code are in `0-prelab-code` (you should copy these into `libunix` at
the top level).

None of them should be very hard; hopefully you can do these before lab:

  1. `read_file(size,name)`: read the file `name` into a buffer you
     allocate in its entirety and return it, writing the size of the
     buffer in `size`.  Implementation goes in `libunix/read-file.c`.

     Note that you should pad the memory with zero-filled data up
     to the next multiple of 4.  (Easiest hack: just add 4 when you
     allocate it with `calloc`.)

     You'll implement this using `stat()` to get the file size,
     and then use `read()` to read the entire file into memory.
     (Don't use `fread()` unless you set it up to read binary!)

     Don't forget to close the file descriptor!  Don't `free` memory
     in general: we'll waste bytes to reduce errors and use `exit`
     as our free.

  2. `find_tty` will look though your `/dev` directory to find the
     USB device that your OS has connected the TTY-USB to, open it,
     and return the resultant file descriptor.  Implementation goes in
     `libunix/find-ttyusb.c`

     You should use `scandir` to scan the `/dev` directory.  (`man
     scandir`).  The array `ttyusb_prefixes` has the set of prefixes
     we have seen thus far; if your computer uses a different one,
     let us know!

     You should give an error if there zero or more than one matching
     tty-serial devices.  There is a `strcatf` helper function that makes
     it easier to create a malloced name.

  3. `find_tty_last`: does the same and finds the most recent TTY-USB.
  4.  `find_tty_first`: does the same and finds the oldest TTY-USB.


### 4. Nice haves to make coding faster, better.

Tags: Again, as discussed in the previous lab, please try to get a "tags"
equivalent installed / used on your development environment.  It will make
looking at function / variable / structure definitions much much easier:
you simply will move the cursor to the identifier you care about, do
some kind of key sequence, and your editor will take you to identifier's
definition.  To pick the first google result for `emacs` and `vim`:
 - [tags in vim](https://vim.fandom.com/wiki/Browsing_programs_with_tags).
 - [tags in emacs](https://www.jayconrod.com/posts/36/emacs-etags--a-quick-introduction)


Multiple code windows: the more different contexts you can keep on
your screen, the less you have to keep in your head and the less time
you have to spend hunting around.  There are a variety of different
terminal programs that make it easy to do split-screen and use hotkeys
to jump between them.  I use `terminator` and am happy with it; there
are many others.  Editors like Vim and Emacs let you create "splits" to see 
different files at once, and programs like `tmux` let you split one terminal 
window into many.

`git` conflicts:  you may wind up having a `git` conflict at some point.
A stupid-simple way if you don't care about your local modifications:

    git checkout -- <file>

See [here](https://docs.gitlab.com/ee/topics/git/numerous_undo_possibilities_in_git/) for
more general variants.

The `guides` directory in this repo contains:
- [a ctags setup guide](../../../guides/ctags.md)
- [a git guide](../../../guides/git.md)
