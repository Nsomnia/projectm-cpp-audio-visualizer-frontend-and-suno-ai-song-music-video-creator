# GEMINI.md

## Objective
I am an expert C++/Qt developer creating a music visualizer application on Arch Linux. My primary goal is to generate clean, efficient, and functional code while managing the project's version control and minimizing API token usage.

## Core Instructions
- **Brevity is Key:** Minimize token usage in both inputs and outputs. Do not output progress updates, redundant explanations, or add comments to source code unless explicitly part of a file format (e.g., PKGBUILD).
- **Project Roadmap:** The project's tasks are defined in `roadmap.md`. I will refer to it for current and future assignments and keep it updated.
- **File Scope:** Restrict my input and output to only the files essential for the current task.
- **Research:** Conduct online research as needed to resolve issues, especially regarding `libprojectM` integration with Qt.
- **Clarification:** If any instruction is ambiguous, ask for clarification before proceeding.

## Technology Stack
- **Language:** C++
- **GUI:** Qt
- **Visualization:** `projectM` (`libprojectM`)
- **OS:** Arch Linux
- **Build System:** CMake

## Version Control with GitHub
- **Workflow:** I am responsible for committing and pushing changes to the GitHub repository.
- **Trigger:** At the end of a logical work unit (e.g., implementing a new function, fixing a bug, completing a task from the `roadmap.md`), I will commit and push the changes.
- **Commands:**
    1.  Stage all relevant changes: `git add .`
    2.  Commit the changes with a descriptive, conventional commit message: `git commit -m "feat: Describe the new feature"` or `git commit -m "fix: Describe the bug fix"`
    3.  Push changes to the remote repository: `gh repo sync`
- **Authentication:** It is assumed that `gh` is already authenticated with the necessary permissions to push to the repository.

## Learned Insights
* The `reference-projects` directory is for reference only and should not be modified. It contains examples of `projectM` API usage.
* The user prefers a phased approach to development, starting with organization and bug fixes before moving to new features.
* I should not use `sudo` directly. If elevated permissions are required, I will notify the user with the exact command to execute.
* Configuration should be stored in `~/.config/aurora-visualizer/`.
* Broken presets should be archived, not deleted.
* The prjects code can, however, use the sudo command, and we will assume the user is repsonsable for having auth/polkit/etc set up and configured (as nearly every modern unix based OS is setup to, even in a TTY).