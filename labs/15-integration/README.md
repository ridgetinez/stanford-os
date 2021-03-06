
Today's lab will do a partial integration of the pieces you've written
into a small OS that has virtual memory and user processes.

I'm still writing a piece of the code, so this might be two small labs
rather than one, but at the very least you'll have a real virtual memory
with single stepping.


For the first couple of parts, we'll start by adding to a copy of your 
lab 10 to keep new stuff to a minimum.  Then we'll flip that code into 
a more full fledged code base.

--------------------------------------------------------------------
Part 0: fix our context saving and restoring!

It turns out there was an interesting bug in our context switching code.
When doing single stepping, we can't simply do:

        ldm r0, {r0-r15}^

to reload user state.  If you look at the ARMv6 document, you'll see
`ldm` behaves differently depending on if you have the `pc` (`r15`)
as one of the registers.

<table><tr><td>
  <img src="images/ldm-pc.png"/>
</td></tr></table>

This bug is a bit interesting in that we missed this nuance for the past
4-5 years but had managed to avoid getting bitten by it despite doing
context switching in different ways b/c of how it was implemented.
We didn't realize the bug in lab 10 because we had no other process
running.

In any case, first step is to fix your code to use `rfe` and (if you
want) `srs` to restore and save state. If you look at the `prelab-code`
directory you'll see an example that uses them.  You'll want to look in 
the ARMv6 manual to make sure you understand.

What to do:
   1. Copy your `10-process/code` into `15-integration/code` (we never
      want to mutate a working system that we've already checked).
   2. Make sure your tests run as before.
   3. Rewrite the equivalance assembly to use the `rfe` instructon 
      at the end.
   4. Make sure your tests still pass!

--------------------------------------------------------------------
Part 1: save state to a process structure

For our OS we'll want to save the current registers into a process
structure rather than onto a random exception stack.  

For this part, put the following trivial process structure in the
`fake-os.h` header along with the external pointer declatation:

    // add this to your fake-os.h
    typedef struct {
        // register save area: keep it at offset 0 for easy
        // assembly calculations.
        uint32_t reg_save[16 + 1];  // 16 general purpose registers + spsr

        // used by the equiv code.
        pix_equiv_t ctx;
    } pix_env_t;

    extern pix_env_t *pix_cur_process;

Add this to your `fake-os.c` at the top:
    
    static pix_env_t init_process;
    pix_env_t *pix_cur_process = &init_process;

You'll make the following changes:

   0. Run your code to make sure it still gives the same checksums.

   1. Make a copy of your assembly and have it load the location to
      save registers to using the `pix_cur_process` variable
      (above).  You might want to look at the old interrupt code (in
      `7-interrupts/timer-int/interrupts-asm.S`) to see how to do this
      easily in assembly.

      As a sanity check: before rewriting the assembly code, first pass in
      the value you loaded above into the single step exception handler
      as an argument and `assert` that it is equal to the value held in
      your `pix_cur_process`.

      Note: you will need to do two `ldr` instructions: one to get the
      value of the global variable `pix_cur_process` and then another to
      load its contents.  You should be able to do an `=pix_cur_process`
      in the assembly for the first one.

      Note: An easy mistake to make is to forget that `r0`, `r1` and
      the other caller saved registers will get trashed by the exception
      handler --- so don't assume their original values are still there!

   2. Make a copy of your equivalance code that stores the current context
      in this process structure and uses it to track the current hash.
      Have the assembly routine from (1) pass a pointer to the current
      context structure in.

   3. Rerun your code and make sure it still gives the same checksums.
      (You'll notice a pattern: this is how I always make changes --
      tiny change, then verify checksum equivalance --- so that I don't
      have to think that hard.)

   4. Initialize your process structure save area so the registers
      in it have the same values as would be set by your
      `user_mode_run_fn`.  

            // reg_save offsets for the different registers.
            enum {
                SP_OFF = 13,
                PC_OFF = 15,
                SPSR_OFF = 16,
            };

            // initialize the register save area
            pix_env_t pix_env_mk(uint32_t pc, uint32_t sp) {
                pix_env_t p = {0};
            
                p.reg_save[0] = pc;
                p.reg_save[1] = sp;
                p.reg_save[SPSR_OFF] = 0x190;
                p.reg_save[PC_OFF] = pc;
                p.reg_save[SP_OFF] = sp;
                return p;
            }

    5. Write a `switchto_asm` routine that takes a pointer to the
       `reg_save` array and loads everything: use this instead of the
       `user_mode_run_fn`.  It should use `rfe` instead of changing
       `spsr` first.

      Then you should be able to call `switchto_asm`:

            switchto_asm(&p->reg_save[0]);

      And have it work equivalantly to:

            user_mode_run_fn((void*)p->reg_save[15], p->reg_save[13]);

   6. Rerun your code to make sure it still gives the same checksums.

   7. Now do `switchto_asm` at the end of the equivalance routine 
      `equiv_run_fn_proc` instead of returning.

   8. Rerun your code to make sure it still gives the same checksums.

   9. Merge all of your assembly (`user-mode-run-fn.S` and `fake-os-asm.S`)
      into a file called `pix-asm.S`

   10. Rerun your code to make sure it still gives the same checksums.

Great: now you have code that will correctly save and restore registers
from a process structure (which we need as a first step for an OS).

--------------------------------------------------------------------
Part 2: Migrate over to pix.

If you do a pull, there should be a new directory at the top of the 
repo: `cs140e-21spr/pix`.   Inside it's modeled on the fake OS we
did in lab 10:

            % ls
            Makefile  pitag-linker	README.md
            mk	  pix-kernel	pix-user	  tests

The directories you'll care about:

   - `pix-kernel`: this holds the kernel code (it's similar to 
      `fake-os`).  This is the only directory you should have to 
      modify anything in.
   - `pix-user`: a very simple user-level library (similar to 
     `fake-user-level`).
   - `tests`: these are the tests from before.

It's more real than the fake-os we used before in that it has virtual memory, process
blocks and tracks segments.

As a first step, you should be able to type `make check` and the 
tests should pass:

        # check hashes
        cs140e-21spr % cd pix
        cs140e-21spr/pix % make check

        checking <1-test-hello>:  Success
        checking <0-test-nop>:  Success
        checking <0-test-exit>:  Success
        checking <3-test-vec>:  Success

Again, these are the same tests from lab 10.  You can also type `make run`
to just run them and display the results to the screen without checking
--- this can make figuring out errors easier.

If you look in the `pix-kernel` directory, there's a bunch of files.
For the most part they are `.o` files of code you've already built (that
you will replace) or `.c` files that provide brain-dead simple versions
of needed functionality so we can run processes.

As a first step, you should migrate your code from above into:

   - `pix-asm.S` : copy it from above into this `pix-kernel`.

     This file should define two exception vectors --- `part4_equiv_vec`
     (this is identical to the same vector in lab 10) and
     `proc_equiv_vec`, which calls the exception handling code you wrote
     above that uses the process control block for state.  It should
     also contain your `switchto_asm`.

   - `equiv.c`: this should contain your equivalance checking code from
      part 1.
 
   - If you remove `staff-pix-asm.o` and `staff-equiv.o` from the `Makefile`
     and put in your versions, `make check` should still pass.

#### Putting it all together.

Here you'll go through a set of steps and make sure that each passes
the checksum tests.


Note: to print the first N do this:

            pix_equiv_t *c = &pix_get_curproc()->ctx;


            if(n < c->print_first_n) {
                trace("\treg hash=%x\n", c->reg_hash);
                trace("\tspsr=%x\n", spsr);
                trace("\tpc = %x, lr = %x\n", regs[15], pc);
                for(int i = 0; i < 16; i++)
                    trace("\tregs[%d] = %x\n", i, regs[i]);
                trace("------------------------------------------------------\n");
            }


For these you'll change the `part` variable in `pix.c` so that it
goes through each part in turn, making sure the test still passes.
(Yeah, this is pretty gross, apologies.)

  1. `part = 1`: this should run your new equivalance code that uses
     the process control block.

  2. `part = 2`: this will switch to using virtual memory.  Make sure
      the test passes, then change all the `staff_mmu_` calls to use you
      routines.  You'll also have to add your `mmu.o` and `mmu-asm.o`
      to the `Makefile`.

  3. `part = 3`: this will switch to using a process control block and 
      cloning the process.  Test should still work.

  4. `part = 4`: this uses the `schedule()` routine to switch proceses
      (a simple round-robin scheduler).
  5. `part = 5`: this will run multiple processes.  As a first step,
      just run it.  It should run the same process multiple times.
      Then, in your `equiv.c` change it to call `schedule()` instead
      of `switchto_asm` when `pix_config.use_schedule_p` flag is set.
      This will switch between processes at every single instruction.
      If you inspect the results of `make run` you should see the
      same checksum for all. 

            void prefetch_abort_equiv_proc(...) {
                ...
                if(pix_config.use_schedule_p)
                    schedule();
                else
                    switchto_asm(regs);
                not_reached();
           }



Congratulations!  This is a very hard set of tests.  At this point you
have rock-solid context switching code and a ruthless way to immediately
detect a different in any register at any instruction from any mistake
that leads to a user-visible change.  You can use this setup to keep
doing many changes with an induction-based safety net.  

Some simple changes:
  0. change the linker to load multiple programs at once.
  1. Change the page size.
  2. Use two page tables (one for system, one for user-level)
  3. Compile `pix` with higher level optimizations.
  4. Turn on caching; switch from write-through to write-back.
  5. Change the expensive virtual memory coherence methods from lab 12
     to be more targeted (and much more efficien).  In particular, don't
     kill the entire TLB when modifying a PTE entry (or defer the coherene
     until the end).
  6. If you turn on interrupts, and change the frequency, there should be
     no user-visible difference.
  7. Load the binaries from the SD card using your FAT32 file system.
  8. Send the binaries over the NRF network.

There's tons of additional ones!   The cool thing is your checksums should
never change.
