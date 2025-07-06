#!/bin/bash

# A script to streamline the git commit and push process.

# --- Configuration ---
# The remote to push to.
REMOTE="origin"
# The branch to push to.
BRANCH="main"

# --- Functions ---
# Print an error message and exit.
function error_exit {
    echo "Error: $1" >&2
    exit 1
}

# --- Main Script ---
# Check if a commit message was provided.
if [ -z "$1" ]; then
    error_exit "Please provide a commit message."
fi

# Add all changes to the staging area.
git add . || error_exit "Failed to add files to the staging area."

# Commit the changes.
git commit -m "$1" || error_exit "Failed to commit changes."

# Push the changes to the remote repository.
gh repo sync "$REMOTE" -b "$BRANCH" || error_exit "Failed to push changes to the remote repository."

echo "Successfully committed and pushed changes."
