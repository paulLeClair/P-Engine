# p-engine-develop

My private development repo for the C++ game engine framework I'm making in my free time

## PLACEHOLDER README

Welcome, me (or hackers)! You know what this is. Basically it's time to reinvent the shitty wheel I currently have, and hopefully I will end up with a superior wheel. 

NOTE: it might make sense to make your own fork if you want to start making changes, so hit me up about that!

## HOW THIS WORKS FOR NOW

I'll write out a detailed document that actually describes the process I'm gonna follow in detail, but here's the overview:

1. individual bits of work are grouped into GitHub **issues**, which are each put into a **project** (basically a [kanban](https://en.wikipedia.org/wiki/Kanban_(development)) board) as well as a **milestone** (which is a way to define the overall chunk of work you're doing, which just consists of completing a bunch of those issues)

2. design documents are (ideally) written **before** any coding starts, describing the classes being written in as much detail as is necessary. Sometimes (especially early on) it'll be hard to write a complete design before starting implementation, but any on-the-fly design should be documented as you go so that there is always something you can read about each part of the code. 
  - the general idea is: first make a **design issue**, then as part of that design issue open an **implementation issue** which is meant to write the code that the design document describes

3. although at this point there isn't a lot of code, this project follows [test driven development](https://en.wikipedia.org/wiki/Test-driven_development) as closely as possible! Any ghosts/hackers can message me for more info on this, there are TONS of benefits and it really helps the development process as well as giving you a powerful workflow to rely on when actually writing code.

This project also follows a simple version of the usual [git](https://en.wikipedia.org/wiki/Git) process (message me for more info on this, it's honestly like 25% of software development) for version control:
  - make a new branch off the main branch, named after the issue you're working on
  - work on that separate branch until you've completed the issue
  - open a [pull request](https://en.wikipedia.org/wiki/Distributed_version_control#Pull_requests) when all tests pass etc
  - merge the branch back into the main branch when all tests pass and close the issue 

I think that covers most of it! I use the [Wiki tab](https://github.com/jabronicus/p-engine-develop/wiki) (it's in the GitHub repo topbar with a book icon) for all the documentation, and hopefully as the project develops there will be a ton of documentation that describes the motivations behind all the code, as well as the designs themselves (including diagrams ideally) as well as a description of the implementation decisions.
