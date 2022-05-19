# SpOCK
Spacecraft Orbital Characterization Kit

## Project organization

- `src/` source for the application
- `lib/` source for the application library (*.cpp *.hpp)
- `include/` interface for the library (*.h)
- `tests/` unit test framework
- `doc/` doxygen documentation)

> :bulb: for stucture explanation, see [this discussion on SO](https://stackoverflow.com/questions/2360734/whats-a-good-directory-structure-for-larger-c-projects-using-makefile)

## Git branching model

We will use the following workflow:

- `main` branch where the source code of HEAD always reflects a production-ready state.
- `develop` branch where the source code of HEAD always reflects a state with the latest delivered development changes for the next release.
- *supporting branches*: Next to the permanent branches main and develop, supporting branches have a limited time life. They aid parallel development between team members, ease tracking of features, prepare for production releases and assist in bug/fix tracking. The different types of branches we may use are:
  - `feature` branches
  - `release` branches
  - `hotfix` branches

> :bulb: The rational of this workflow is described in [this article](https://nvie.com/posts/a-successful-git-branching-model/).

![alt text](../media/git-branching-model.png?raw=true)

## How to store images and medias

For the use of media (images, videos etc) in the README, we created an empty `media` orphan
branch where the resources are uploaded to. This allows the master branch to be focused
on code.

> :bulb: This usage is documented [here](https://medium.com/@minamimunakata/how-to-store-images-for-use-in-readme-md-on-github-9fb54256e951).
