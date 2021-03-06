Lab: automatically cross-check your pi code against everyone else's.
======================================================================

***As usual, start with the [PRELAB](PRELAB.md)!***

A goal of this course is that you will write every single line of
(interesting) low level code you use.  On the plus side, you will
understand everything and, unlike most OS courses, there will not be
a lot of opaque, magical code that you have no insight into, other than
sense of unease that it does important stuff beyond your ken.  However, an
obvious potential downside of writing the low-level code that everything
depends on: a single bug in it can make the rest of the quarter miserable.

This lab will have you build the initial pieces needed to automatically
check that your pi code is correct.  We will use the approach to check
that both your GPIO and your bootloader implementations  are equivalant
to everyone else's implementation.

The basic idea: After completing the lab you will:

   1. Be able to check that your `gpio` code is equivalant to everyone
      else's in the class by tracing all reads and writes it does and
      comparing them to everyone else's implementation.  If even one person
      gets it right, then showing equivalance means you got it right too.
      And, nicely, automatically detect if any subsequent
      modifications you do break the code (e.g., if you rewrite
      it to be cleaner).

   2. Extension: Be able to quickly, correctly write a more full-featured
      `gpio` implementation by checking its equivalance against the fairly
      complicated code used in the initial cs107e class.

#### Sign-off

Pass the tests for both parts of thelab:

   1. `1-fake-pi`: You get the same checksum as everyone else for
      all the `.out` files produced by the tests in `1-fake-pi/tests`.

   2. `2-trace`: You get the same checksum for all the `.out` files
      produced by the tests in `2-trace/tests`.

----------------------------------------------------------------------
#### PRELAB review: how to make a fake pi emulator

To recap the PRELAB: In order to make testing easy, we want to be able
to run your r/pi code --- unaltered --- on your Unix laptop.  In general
you'll have to do the following:

  1. Compile it with your Unix compiler (`gcc`).

     Because of how we wrote ` gpio.c` you can easily compile it both
     for RPI and to run on your Unix laptop by simply switching compilers.

  2. Override what happens when it uses ARM assembly code, since your laptop
     (likely) cannot execute this.  

     Fortunately, your GPIO does not use assembly directly; it does call
     the assembly functions `put32` and `get32`, but since we replace them
     (below), this is not an issue.

  3. Emulate reads or writes of GPIO memory addresses --- these will
     likely be illegal on your laptop and will definitely behave
     differently.  (You can check legality by, for example, executing
     `*(volatile unsigned *)0x20200000 = 4` and see if you get a
     segfault!).

     In this case, all you'll have to do is provide fake implementations
     of `get32` and `put32` so that we can override what happens when
     your code writes to GPIO memory.

Essentially, this is all based on the observation that so far the only
real thing we ever _do_ in terms of I/O on the Pi is access memory--and
even then, we only ever do that using the functions `put32` and `get32`.
This means, rather than simulating every single thing going on within the
Pi, we can simply simulate `put32` and `get32` and still get a simulator
that's really similar to the actual behavior on the Pi.  We of course
still need to write some logic around those two functions--some of the
`put32`s go to RAM, others to GPIO, etc.--but these two primitives can
simulate our entire program.

Note that there's a tradeoff here--we can simulate things at different levels 
of complexity, and in return we get different levels of accuracy.  The *most* 
accurate simulation would be to simulate the actual hardware on the Pi, but 
doing so would be a monumental effort.  However, since we only care about the 
I/O interactions via `put32` and `get32`, we don't have to bother simulating 
anything lower-level than that; the code compiled for your laptop is close 
enough to the code compiled for the Pi.  Later we'll want to simulate and test 
more things (timers, specific hardware devices like the serial port, etc.), and 
we'll run into the same tradeoff again.

----------------------------------------------------------------------
#### 0. make sure your `CS140E_2021_PATH` is defined.

From the `PRELAB`, make sure your path is setup:

        % cd 2-cross-check/00-hello
        % make
        % pi-install hello.bin
        listening on ttyusb=</dev/ttyUSB0>
        hello: things worked!
        DONE!!!

If you get a `make` error, your path is not defined. 


If not, make sure you have sourced your shell's config file.  (If you
have a lot of terminals, logging in and logging out might be easier.)

If you have a pi error, there's a hardware issue to take care of.

Now make sure that the entire lab compiles:

        % cd 2-cross-check
        % make

It should work.


Also:

  - Confirm that your GPIO code uses `get32` and `put32`
     instead of a raw read or write of device memory.  So, for example

            *gpio_fsel0 &= ~111b;

     would become:

        put32(gpio_fsel0, get32(gpio_fsel0) & ~111b);


----------------------------------------------------------------------
#### 1. Make a fake implementation of `put32` and `get32`

For this lab and the homework, we need to be able to override what
happens when your code writes to GPIO memory.  Later in this course
(when we build virtual memory), you will use memory protection tricks to
intercept all loads and stores and cause them to do interesting things.
For the moment, we will do things in a much simpler, though manual way.

You'll now build a fake memory so that you can implement `put32` and
`get32`.  

First:
  - Look in `1-fake-pi/fake-put-get.c` and read the comments.  

For `put32(addr,v)`: 
  1. Create an entry for `addr` in your fake memory if `addr` doesn't exist.
  2. Write `v` to `addrs` entry.
  3. Print out these values using the `trace` call.

On `get32(addr)`:

  1. If `addr` does not exist, insert `(addr, random())` (but do not
     print anything).
  2. Get the value `v` associated with `addr`.
  3. Print out these values using the `trace` call.
  4. Return `v`.

To test it:

  0. Read [1-fake-pi/README.md](1-fake-pi/README.md) and 
     [1-fake-pi/tests/README.md](1-fake-pi/tests/README.md)

  1. `make runall` should run the first set of simple put/get tests
     so you can see the output.
     It should run without crashing and, importantly, print out the
     values for each `put32` and `get32` in the exact order they happened.

     `make emitall` will emit thier output to `.out` files (there
     are some `.ref` files for reference).  You can checksum these.

     Don't be afraid to look around the `Makefile` to see what it is doing!

  2. Compare the cksum to your partner.
  3. If these values match, you know your code worked the same as your partner's.
  4. Now post to the newsgroup so everyone can compare.
  5. If everyone matches, and one person got it right, we've proven that
       everyone has gotten it right (at least for the values tested).

----------------------------------------------------------------------
#### 3. Check your code against everyone else (5 minutes)

After you checked your fake `put32` and `get32` we now want to check that
your `gpio` code works the same as everyone else.  Given the `get32` and
`put32` modifications above, a simple, stringent approach is to check
that two `gpio` implementations are the same:

  1. They read and write the same addresses in the same order with
  the same values.
  2. They return the same result.    (For our implementations we
  did not return any result, so this just means that your code
  never crashes.)

If both checks pass then we know that both implementations are equivalent
--- at least for the tested inputs.

For this section start with the simplest case:
 1. Start adding the rest of the tests to `TEST_SRC` in
    `1-fake-pi/tests/Makefile`.  Start with the easier ones and go
    further.    The `prog-*.c` files are the most complicated.

 2. Check that each function gives the same answer as everyone else.

----------------------------------------------------------------------
#### 4. Add `gpio_set_func` and cross-check it.

You'll see we can write an implementation even without having a way to run it.

   1.  Implement the `gpio_set_func` function: it has a prototype 
       in `libpi/include/gpio.h` at the top-level directory.
   2. Add a test to the test harness.
   3. Cross-check your results.

----------------------------------------------------------------------
#### 6. Do similar tracing on the pi 

This uses the tracing trick from the `PRELAB`.  You should look at that
implementation if you haven't already.

Implement the code in:

  - `trace-simple.c`: implement `__wrap_PUT32` and `__wrap_GET32`
  - `trace-notmainc.c: if you want to get fancy implement this 
    so you can run raw pi programs in tracing mode.

As with `1-fake-pi` start working through the tests in `2-trace/tests`.

##### Background

We would also like to check that the code running in its native
environment works as expected: running on your laptop is great, but it
does not guarantee the code works on the pi.

In addition, while running in a fake environment lets us test all sorts of
crazy combinations, it does require extracting the code.  The most common
pain point I've had at least when doing OS stuff is:
  1. You write some device driver.
  2. It takes forever to get the thing working (bad docs, bad programmer, bad etc).
  3. When it finally does it looks like a mess, but you don't want to touch it.

The ability to use read-write equivalance to show that a new version is identical
to an old version allows you to fearlessly fix the disgusting code you wrote as
a first draft and have a safety net to prevent breaking what wasn't already so.

In some sense our problem is easier here: we have a running machine, so
we don't need to make fake anything.  Our main issue, besides some subtle
recursion reasoning (later), is that we want to interpose on calls to
`put32`, `get32` etc so that they first go into our tracing sytem, which
can then call the original functions itself after logging what it needs.
(In the future, you'll want to interpose on a wide set of things.)

One option would be to abuse the C preprocessor to translate the names;
another would be to just go in and replace all the calls with a function
pointer that we can redirect.   There's not that much code yet, so it's
not horrible to do either.  But the GNU linker gives us a more elegant
and safe method of "wrapping".  Assume we want to wrap `GET32`:

  1. When linking the pi program, add a `-wrap=GET32` argument to the linker command.
  2. The linker will replace all calls to `GET32` with calls to `__wrap_GET32`.  We
     will implement the `__wrap_GET32` function.
  3. The linker will also rename the definition of `GET32` as `__real_GET32`, which
     our code can call when (if) it wants to invoke the original code.
  4. The code in `02-trace-pi has an example.


##### Extensions:

  1. Extend the tracing to handle PUT32, put32, and get32.  (both `put32` and
     `PUT32` do the same thing, they just have a different types for their
     address argument, which you can see in `rpi.h`)

  2. Extend the tracing to simply capture reads and writes for later printing.
     This allows us to trace `printk` or anything else that uses the UART.

----------------------------------------------------------------------
#### 5. Replace our `gpio.o` entirely!
  
Now you'll swap in your `gpio.c` for the one in the `libpi.a`.  This should
just take a couple of minutes and then the lab is complete:

   0. `cd 2-trace/tests`
   1. `make emitall` to make sure you have `.out`s for everything.
   2. copy your `gpio.c` to `2-trace/tests/gpio.c`.
   3. uncomment `# GPIO_OBJ = ./gpio.o` in `2-trace/tests/Makefile`
   4. `make clean; make`
   5. `make checkall` --- this will compare the old output (using our gpio.c)
      with the new output (using yours).
   6. Congratulations!  You have removed a big chunk of our code.

----------------------------------------------------------------------
#### Extensions

Making the code more useful mostly involves expanding it so that
even when code writes values in different orders, you can still
show equivalance.

Modify your code to:

  1. Check that your opposing blink code is identical to your
     partners (you will need to make it loop a finite number of times).
     What should you do about `delay`? 

  2.  Show that it's equivalent even when you set output pins
     in different order.  Hint: you likely want to be able
     easily mark some memory as "last writer wins" where it doesn't
     matter the actual order, just the final value.   I would
     do this by tracing each function, automatically marking the
     addresses it writes as last-writer addresses.

  3. How to show equivalance when you set and clear in different orders?
     This is trickier.  You will need to come up with a clean scheme to
     indicate that non-overlapping bit-writes do not interfere.

  4. Within a device, often you can write some fields in any order, and
     there is a final "commit" location you write to turn the device on.
     Devise a good way to cleanly indicate these differences.

  5. Strictly speaking, we need memory barriers when writing to
     different devices.  Add support for checking this.

If you can do all of these you are in great shape to check some
interesting code.
