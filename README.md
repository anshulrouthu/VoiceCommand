VoiceCommand
=======

A voice command utility using Google Speech API. 

Features
=========
- Listen to your command only when you are speaking. (no need to press any key to activate listening)
- Currently only speech to text is implemented.

Build Instructions
==================

$ ./setup.sh (you need to run this only one time per checkout)<br>
$ make all<br>

Testing:<br>
$ ./bin/unittests (for running a unittest on the framework)<br>

Usage
=====
$ ./bin/voiceCommand (and speak, it will print the transcript on console)<br>

If you do not see anything printin on console, try to adjust your microphone volume level.

TODO
====
- Implement Text Processor module
- Implement Command Handler module

Known Issues
============
- There is still some room for optimization. You may miss sone words if speaking a really long sentance.
