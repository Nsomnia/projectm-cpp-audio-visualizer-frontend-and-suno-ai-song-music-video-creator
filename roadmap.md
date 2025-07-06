# Project Roadmap

This document outlines the planned features and improvements for the Aurora Visualizer.

## Phase 1: Core Functionality & Bug Fixes

- [ ] **Playback Logic:**
    - [ ] Implement state management for the play/pause button.
    - [ ] Ensure the next song in the queue plays automatically when the current one finishes.
    - [ ] Implement functionality for the previous/next song buttons.
- [ ] **Visualizer Control:**
    - [ ] Disable automatic preset switching by default.
    - [ ] Add GUI buttons for manual preset switching (previous/next).
    - [ ] Add a settings option to re-enable automatic switching with a configurable interval.
- [ ] **Song Title Animation:**
    - [ ] Fix the fade-in/out animation for the song title.
    - [ ] Refine the "bounce" animation for smoother, more precise movement.
- [ ] **Fix GitHub Link:** Correct the incorrect GitHub link in the "Help" section.

## Phase 2: Advanced Features & Configuration

- [ ] **Configuration Management:**
    - [ ] Save and load settings from a persistent configuration file in `~/.config/aurora-visualizer/`.
    - [ ] Ensure all applicable command-line options have corresponding GUI options.
- [ ] **Broken Preset Management:**
    - [ ] Implement automatic detection of broken `.milk` presets from error output.
    - [ ] Change the "Delete broken preset" button to "Archive preset".
    - [ ] Move broken presets to an archive directory (`~/.config/aurora-visualizer/broken_presets`).
- [ ] **Video Recording:**
    - [ ] Implement video recording functionality.

## Phase 3: Future Enhancements

- [ ] **Lyrics Display:**
    - [ ] Implement karaoke-style lyrics display.
    - [ ] Extract lyrics from Suno.ai song pages.
    - [ ] Initially, display lyrics without precise time synchronization.
    - [ ] **Future:** Explore STT for automatic timestamp generation.
- [ ] **Suno.ai Integration:**
    - [ ] Implement batch downloading of songs from a list of URLs.
    - [ ] Improve download management with error handling and progress reporting.
