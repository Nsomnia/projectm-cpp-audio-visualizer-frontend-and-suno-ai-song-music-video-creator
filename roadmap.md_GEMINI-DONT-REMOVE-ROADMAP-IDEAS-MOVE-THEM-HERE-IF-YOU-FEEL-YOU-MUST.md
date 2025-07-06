# Aurora Visualizer

A highly-customizable desktop GUI audio visualizer designed for musicians and content creators to generate unique, audio-reactive music videos.

![placeholder](https://i.imgur.com/gY8g4i2.png)

---

## Core Features

*   **Real-time Audio Visualization:** Renders classic and modern MilkDrop-style presets using the projectM engine.
*   **Dynamic Text Overlays:** Displays song titles and artist information with engaging animations.
*   **Advanced Animation Control:**
    *   Text fades in, bounces around the screen, and fades out in sync with the song's duration.
    *   A subtle "breathing" effect adds a dynamic feel to the text.
*   **Intelligent Text Wrapping:** Song titles are automatically wrapped to create balanced, readable lines.
*   **Audio-Reactive:** Visualizations react dynamically to the frequency and beat of the currently playing audio.
*   **Wide Audio Format Support:** Plays `.mp3`, `.wav`, and `.flac` audio files.
*   **Full Keyboard Control:**
    *   `N` key: Switch to the next random preset.
    *   `P` key: Switch to the previously viewed preset.

---

## Prerequisites (Arch Linux)

```bash
sudo pacman -S --needed git make gcc cmake qt6-base projectm freetype2
```

---

## Build & Run Instructions

1.  **Clone the repository:**
    ```bash
    git clone <your-repo-url>
    cd <your-repo-directory>
    ```

2.  **Build the application:**
    ```bash
    make
    ```

3.  **Run the application:**
    ```bash
    ./build/AuroraVisualizer [options]
    ```

### Command-Line Options
*   `--default-preset` or `-d`: Start with a fixed preset for debugging.
*   `--artist "Artist Name"` or `-a "Artist Name"`: Set the artist name to be displayed.
*   `--url "https://your-url.com"` or `-u "https://your-url.com"`: Display a static URL in the top-left corner.
*   `--font "/path/to/font.ttf"` or `-f "/path/to/font.ttf"`: Specify a custom font for text rendering. This overrides the font path in `config.ini`.
*   `--font-size <size>`: Set the font size.
*   `--title-line-length <length>`: Set the target line length for song titles.
*   `--title-color-r <value>`: Set the red component of the title color (0.0-1.0).
*   `--title-color-g <value>`: Set the green component of the title color (0.0-1.0).
*   `--title-color-b <value>`: Set the blue component of the title color (0.0-1.0).
*   `--title-opacity <value>`: Set the opacity of the title (0.0-1.0).
*   `--fade-duration <seconds>`: Set the fade duration for animations.
*   `--bounce-duration <seconds>`: Set the bounce duration for the title animation.
*   `--target-alpha <value>`: Set the target alpha for the title animation (0.0-1.0).

---

## Configuration

The visualizer can be configured via a `config.ini` file. The application will first look for this file in `~/.config/aurora-visualizer/config.ini`. If it's not found, it will fall back to the `config.ini` in the application's directory.

### Example `config.ini`
```ini
# Default configuration for Aurora Visualizer
# User-specific overrides can be placed in ~/.config/aurora-visualizer/config.ini

[Font]
# Path to the TTF font file for rendering text.
path = /usr/share/fonts/TTF/DejaVuSans.ttf
# Base font size for the song title.
size = 48

[Title]
# The target character length for each line of the song title.
# Lower values result in more vertical stacking.
line_length_target = 20
# Color of the title text (RGB, 0.0 to 1.0).
color_r = 1.0
color_g = 1.0
color_b = 1.0
opacity = 1.0

[Animation]
# Duration of the fade-in and fade-out effects in seconds.
fade_duration = 3.0
# Duration the title bounces around before starting to fade out.
bounce_duration = 10.0
# The target opacity (alpha) for the text after it fades to transparent.
target_alpha = 0.4
```

---

## Future Development Roadmap

This project aims to become a complete solution for automated music video production.

### Immediate Goals
*   **UI/UX Enhancements:**
    *   Implement a settings window to control all visual and audio parameters.
    *   Add a menu bar with options for file operations, playback control, and help.
    *   Create a playback queue for songs.
*   **Video Recording:**
    *   Integrate `FFmpeg` to record the visualizer output, including text overlays, to a video file.
    *   Develop a robust file-saving mechanism to prevent overwriting existing videos.
*   **Preset Management:**
    *   Add a UI element to easily delete broken or unwanted `.milk` preset files.

### Long-Term Vision
*   **Advanced Text Effects:**
    *   Support for custom GLSL shaders for text animations.
    *   Karaoke-style lyric synchronization using speech-to-text engines.
*   **suno.ai Integration:**
    *   Automate downloading songs and lyrics from suno.ai for seamless video creation.
*   **Community Features:**
    *   Create a system for users to share and download presets, animation templates, and color schemes.

---
*This README was last updated by the Gemini CLI.*
