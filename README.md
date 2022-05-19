# SpOCK
Spacecraft Orbital Characterization Kit

## Project organization

- `src/` source for the application
- `lib/` source for the application library (*.cpp *.hpp)
- `include/` interface for the library (*.h)
- `tests/` unit test framework
- `doc/` doxygen documentation)

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

> :bulb: We use [Release Drafter](https://github.com/marketplace/actions/release-drafter) to automate the version labeling, feature trakcking and documentation. Please use informative labels during your PR drafting!

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
