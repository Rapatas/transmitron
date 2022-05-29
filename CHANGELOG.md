# Changelog

## [0.0.3] - 2022-05-29

### Added

- Auto complete topics for publish and subscribe fields.
- Command line argument to launch directly into a profile.
- Layout field in profile options.
- Keyboard shortcut to close tab (ctrl-w).
- Keyboard shortcut to open new tab (ctrl-t).
- Label to the store message button.
- Label to the layouts drop-down menu.

### Fixed

- Tab key navigating out of order.
- Broker timeouts parsed as milliseconds.
- Retained option applied when selecting a retained message.
- Not allowing to remove subscribed topic while disconnected.
- Button padding clipping icon on some GTK themes.

### Changed

- Default timeout values set to 5 seconds.
- When creating a new profile, start editing the name field.
- Use Hostname as client ID when none is provided.

## [0.0.2] - 2021-11-05

### Added

- Show selected snippet name on publish editor.

### Fixed

- Black caret when using dark background.
- Snippet remained highlighted when selecting a message from history.
- Missing background color for lexical error in editors.

## [0.0.1] - 2021-10-21

### Added

- Line numbers in editors.
- Header and banner in NSIS installer.
- Save icon in profile save button.
- Label on format drop-down of editors.

### Fixed

- Crash when subscribing to an invalid topic.
- Package description for `.deb` installer
- Overwrite snippet context option appearing for folders.

## [0.0.0] - 2021-09-18

### Added

- **Profiles.** Store connections to brokers.
- **Multiple Connections.** Connect to multiple `Profiles` at the same time using tabs.
- **Snippets.** Store messages in a nested folder structure, ready to publish.
- **Folding.** For messages with nested data.
- **Syntax highlight, detection & formatting.** Supports JSON and XML.
- **Flexible.** Resize, drag, detach or hide each sidebar separately.
- **Layouts.** Store sidebar locations and sizes.
- **XDG BaseDir.** Respects the [XDG Base Directory](https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html) specification.
- **Native UI.** Using `wxWidgets` to integrate seamlessly with your desktop theme.
- **Mute / Solo.** Hide or isolate messages for each subscription.
- **History Filter.** Limit history using search terms.
- **Cross-Platform.** Built for Windows and Linux.
