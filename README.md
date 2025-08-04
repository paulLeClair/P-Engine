# p-engine-develop

My private development repo for the C++ game engine framework I'm making in my free time.

>2024 update! We're now bringing some folks in (for educational purposes, mainly); welcome, honored guests 📿🙏

## PLACEHOLDER README

Welcome, me (or hackers)! You know what this is. Basically it's time to reinvent the shitty wheel I currently have, and hopefully I will end up with a superior wheel. 

NOTE: it might make sense to make your own fork if you want to start making changes, so hit me up about that!

## HOW THIS WORKS FOR NOW

I'll write out a detailed document that actually describes the process I'm gonna follow in detail, but here's the overview:

BASICALLY I HATE AGILE NOW. And don't even say the word "scrum" to me unless you want to see me blow chunks or you're talking about rugby (in which case, scrum away).

The new process is very simple:
* think about what needs to be done next
* figure out how to do it, learn any concepts required to understand the process of doing it
* do it
* repeat

Along the way it's good to be detailed, but you should end up with a solid body of code that's SIMPLE AS HELL and readable. 

A few other ground rules I'm trying to follow:
* minimal coupling between classes
* shallow inheritance only (wherever possible)
* don't neglect functional programming
* classes should be simple and have very well-defined responsibilities
* the overall structure of the code should follow an identifiable pattern, for better or worse

This project also follows a simple version of the usual version-control-with-[git](https://en.wikipedia.org/wiki/Git) process (message me for more info on this, it's honestly like 25% of software development) for version control:
  - make a new branch off the main branch, named after the issue you're working on
  - work on that separate branch until you've completed the issue
  - open a [pull request](https://en.wikipedia.org/wiki/Distributed_version_control#Pull_requests) when all tests pass etc
  - merge the branch back into the main branch when all tests pass and close the issue 

I think that covers most of it! I use the [Wiki tab](https://github.com/jabronicus/p-engine-develop/wiki) (it's in the GitHub repo topbar with a book icon) for all the documentation, and hopefully as the project develops there will be a ton of documentation that describes the motivations behind all the code, as well as the designs themselves (including diagrams ideally) as well as a description of the implementation decisions.
