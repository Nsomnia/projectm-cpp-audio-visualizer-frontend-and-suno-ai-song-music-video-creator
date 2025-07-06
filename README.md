# Aurora Visualizer

A highly-customizable audio visualizer built with Qt and projectM.

## Features

*   **Real-time Audio Visualization:** Renders classic and modern MilkDrop-style presets using the projectM engine.
*   **Audio Playback:** Supports various audio formats (MP3, WAV, FLAC).
*   **Song Queue:** Manage and play a list of songs.
*   **Video Recording:** Record visualizations to video files.
*   **Configurable Settings:** Customize font, resolution, and other parameters.

## Keyboard Shortcuts

*   `N` key: Switch to the next random preset.
*   `P` key: Switch to the previously viewed preset.
*   `Space` key: Play/Pause audio playback.

## Building and Running

Before submitting any changes, it is crucial to validate them by running the full preflight check. This command will build the repository, run all tests, check for type errors, and lint the code.

To run the full suite of checks, execute the following command:

```bash
npm run preflight
```

This single command ensures that your changes meet all the quality gates of the project. While you can run the individual steps (`build`, `test`, `typecheck`, `lint`) separately, it is highly recommended to use `npm run preflight` to ensure a comprehensive validation.

## Contributing

Contributions are welcome! Please adhere to the existing coding style and project conventions. Before submitting a pull request, ensure all tests pass and the codebase is lint-free.

For a detailed list of planned features and improvements, please see the [Roadmap](roadmap.md).