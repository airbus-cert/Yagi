# 1.4.0
* Upgrade to Ghidra 10.1
* Add support for z80
* Improve variable management (typing and naming)
* Fix issues on T9 register MIPS compiler optim
* Fix access violation in Ghidra (now use a fork to track local changes)

# 1.3.0
* Add action do deal with T9 register MIPS compiler optim
* Add rule to deal with Control Flow Guard
* Fix installer path for Ida Freeware
* Enable decompilation for x86 16 bits in Real Mode #11
* Allow install into another folder than the main one, thanks @jbcayrou

# 1.2.0
* Add support for 6502
* Fix stack overflow #7 during solve of type tree (cyclic)

# 1.1.0
* Add support for Atmel processor (#4)
* Change shortcut for decompilation now is :warning: **F3** :warning: (#5)
* Add clear shortcut to clear type definition for local var
* Change paradigm for findings symbols

# 1.0.1
* Bug fix in type caching