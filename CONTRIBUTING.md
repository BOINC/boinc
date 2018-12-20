# Contributing to BOINC
:+1: First off, thank you for taking the time to contribute! It's people like you that make BOINC such a great tool. :+1:

## How Can I Contribute?

### Helping translate
See: https://boinc.berkeley.edu/trac/wiki/TranslateIntro

### Reporting Bugs
Did you find a bug? Make a Bug report.
* Ensure the bug was not already reported by searching on GitHub under [Issues](https://github.com/BOINC/boinc/issues).
* If you're unable to find an open issue addressing the problem, [open a new one](https://github.com/BOINC/boinc/issues/new).

### Suggesting Enhancements
Do you have a feture idea? Make a Feature request.
* Ensure the feature was not already requested by searching on GitHub under [Issues](https://github.com/BOINC/boinc/issues).
* If you're unable to find an open issue addressing the idea, [open a new one](https://github.com/BOINC/boinc/issues/new).

### Making Pull Requests
**1. Fork & create a branch**

[Fork BOINC](https://help.github.com/articles/fork-a-repo/) and create a branch with a descriptive name.
A good branch name would be:
```
git checkout -b update-code-of-conduct
```

**2. Implement your fix or feature**

At this point, you're ready to make your changes! Feel free to ask for help, everyone is a beginner at first. :wink:

Development Practices:
* Code has followed the [style guide for BOINC](http://boinc.berkeley.edu/trac/wiki/CodingStyle)
* Commit does not contain unrelated code changes
* Code uses [atomic commits](https://www.freshconsulting.com/atomic-commits/)

**3. Sync changes made in the original repository with your fork**
At this point, you should switch back to your master branch and [make sure it's up to date with BOINC's master branch](https://help.github.com/articles/configuring-a-remote-for-a-fork/):

```
git remote add upstream https://github.com/BOINC/boinc.git
git checkout master
git pull upstream master
```

Then update your branch from your local copy of master, and push it!

```
git checkout update-code-of-conduct
git rebase master
git push --set-upstream origin update-code-of-conduct
```

**4. Make a Pull Request** 

Finally, go to GitHub and [make a Pull Request](https://help.github.com/articles/creating-a-pull-request-from-a-fork/).

## Styleguides

### Git Commit Messages
* Use the present tense ("Add feature" not "Added feature")
* Use the imperative mood ("Move cursor to..." not "Moves cursor to...")


Thanks! :heart:

BOINC Team
