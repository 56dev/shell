
# Shell Reimplementation (STATUS: BASE FINISHED)
[Meaning: several issues still exist, and opportunities to optimize, but I'm done with the base features]

This is being created loosely following the [CodeCrafters Build your own Shell](https://app.codecrafters.io/courses/shell/overview) project, but independently developed and containing several of its own features. Despite the name, this shell is designed more like the Windows Command Line rather than the Unix shell. It can only be executed on Windows as well, because I designed it to use the Win32 API heavily. 

As of August 27, 2025: ~400 LOC.

## Commands Included

- echo 
- cd 
- mkdir 
- exit
- type (determine if command exists)
- pwd (print working directory)
- dir (list contents of current directory)

## Other Features

Able to search through PATH.
Able to execute command-line EXEs (GUI => see Issues).
