# SpOCK
Spacecraft Orbital Characterization Kit

[![Website](https://img.shields.io/website?down_message=documentation%20offline&up_message=documentation%20online%21&url=https%3A%2F%2Fbecheler.github.io%2FSpOCK%2F)](https://becheler.github.io/SpOCK/)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
![Lines of code](https://img.shields.io/tokei/lines/github/Becheler/SpOCK)

## Installation

### With Conan + CMake

#### 1 - Get Conan and CMake

[Conan](https://conan.io/) is one of the leading options for cross-platform package
manager for C/C++ projects. We chose it because it interfaces with CMake in a nice
way. Conan will handle the dependencies and version conflicts management, and pass
the paths of the installed dependencies to CMake so it can build the project.

[CMake](https://cmake.org/cmake/help/latest/manual/cmake.1.html) is the C++ build systems first choice for cross-platform development. Technically, CMake is a build system generator but the
level of abstraction it offers allows us to consider it as a cross-platform build system.
Users can build, test, and install packages with `cmake` and `ctest` commands.

> :bulb: For devs, if you want more background on how Conan and CMake interact, check:
> - [the CMake official documentation](https://docs.conan.io/en/1.36/integrations/build_system/cmake.html)
> - [this post](https://jfreeman.dev/blog/2019/05/22/trying-conan-with-modern-cmake:-dependencies/).

##### Configuring the dependencies :wrench:

Lets start in the project root folder:

```bash
$ mkdir build && cd build
$ cmake -D CMAKE_BUILD_TYPE=Release \
        -D CMAKE_C_COMPILER=/usr/bin/gcc-10 \
        -D CMAKE_CXX_COMPILER=/usr/bin/g++-10 \
        ..
```

#### 3 - Build, test, install :rocket:

```bash
# Works on Linux, OSX, and Windows.
$ ncpus=$(python -c 'import multiprocessing as mp; print(mp.cpu_count())')
$ cmake --build . --parallel ${ncpus}
$ ctest --parallel ${ncpus}
$ cmake --build . --target install
```

## Project API documentation

The project C++ library documentation is built with [Doxygen and automatically published on Github Pages](https://becheler.github.io/SpOCK/).

## Project organization

- `src/` source for the main application (SpOCK program)
- `include/spock_lib` interface for the **spock_lib** header-only library (*.h)
- `tests/` unit test framework
- `doc/` doxygen documentation

> :bulb: for structure explanation, see [this discussion on SO](https://stackoverflow.com/questions/2360734/whats-a-good-directory-structure-for-larger-c-projects-using-makefile)

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

## Contributing

If you want to contribute to this project, the simplest way is to:
- Fork it.
- Clone it to your local system with `git clone XXX.git`
- Make a new branch with `git checkout -b feature`
- Make your changes with `git commit -am "updated code for new feature"`
- Push it back to your repo `git push`
- Click the Compare & pull request button.
- Click Create pull request to open a new pull request.

### Pull Request format

We use [Release Drafter](https://github.com/marketplace/actions/release-drafter) to automate the version labeling, feature tracking and project documentation. Please use informative labels during your PR drafting:

- PRs with that belong to the *Features* category should be labeled with:
  - **feature**
  - **enhancement**
- PRs that belong the *Bug Fixes* category should be labeled with:
  - **fix**
  - **bugfix**
  - **bug**
- PRs that belong to the *Maintenance* category should be labeled
  - **chore**

### Conventional Commits Specification

The [Conventional Commits specification](https://www.conventionalcommits.org/en/v1.0.0/) is a lightweight convention on top of commit messages. It provides an easy set of rules for creating an explicit commit history; which makes it easier to write automated tools on top of.

The commit message should be structured as follows:
--------------------
```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```
----------------------
Defining commit `<type>`
- `fix:` for a commit that patches a bug (this correlates with **PATCH** in Semantic Versioning).
- `feat:` for a commit that introduces a new feature (this correlates with **MINOR** in Semantic Versioning).
- `chore:` for a maintenance commit
- `docs:` for a documentation update
- `test:` for adding/removing/updating unit tests
- `refactor:` for commits that do not add or remove any feature, but modifies the code design.

## Automated testing

We use a Github Action described in `.github/worflows/cmake.yml` to automatically:
- Install SpOCK dependencies
- Configure CMake
- Build the SpOCK executables
- Test the project using ctest
- Generate the Doxygen documentation
- Deploy the documentation to the Project Github Page

## How to store images and medias

For the use of media (images, videos etc) in the README, we created an empty `media` orphan
branch where the media resources are uploaded to. This allows the master branch to be focused on code.

```
git checkout media
git add XXX.png
git commit -m "adding XXX.png to media branch"
git push origin media:media
git checkout main
```

In the README on branch `main`, the picture can then be referred as `![alt text](../media/XXXX.png?raw=true)`

> :bulb: This usage is documented [here](https://medium.com/@minamimunakata/how-to-store-images-for-use-in-readme-md-on-github-9fb54256e951).
