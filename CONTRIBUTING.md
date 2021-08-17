# Contributing
**Fly Communty** say THANKS for your interest in advance.

Here you can find more details to work in FLy Project,
there are two primary ways to help:

- _Opening Issues_
- _Doing Code changes_

You can also _Posting in Discussions_ for all arguments which not need to be tracked.

## Opening Issues
Use the [issue tracker](https://github.com/fly-lang/fly/issues) to suggest feature requests, report bugs, and ask questions.
This is also a great way to connect with the developers of the project and getValue
confirmation of your bug or approval for your feature request this way before starting to code.

## Doing code changes
### Fork & create a branch
If this is something you think you can fix, then fork Active Admin and create a branch with a descriptive name.
A good branch name would be (where issue #473 is the ticket you're working on):
```sh
git checkout -b 473-add-feature-name
```

### Get Test running
Before commit check always if project execute and all tests not fails.
If you add new feature, you need to create a unit parseArgs.

### Make a Pull Request
You should switch back to your master branch and make sure it's up to date with Fly's master branch:
with enough details in the comment to understand your work.
```sh
git remote add upstream git@github.com:fly-lang/fly.git
git checkout master
git pull upstream master
```

Then update your feature branch from your local copy of master, and push it!

```sh
git checkout 473-add-feature-name
git rebase master
git push --set-upstream origin 473-add-feature-name
```

Finally, go to GitHub and [make a Pull Request](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/creating-a-pull-request) :D

## Posting in Discussions
If you want to share a comment with Community and you are a member,
you can do a post Discussions, by speaking about project but not related to code or not need to be tracked.
Please before post do a search to look for if your argument was already posted, 
if not chose a right categories:
- General: if you can't insert your post in the following categories
- Idea: if you think something of new
- Q&A: post here your questions, or answer to Community
- Show and tell: tell to Community about a topic
