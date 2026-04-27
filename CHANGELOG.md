# Changelog

All notable changes to `ossim-gui` are documented here.

## [2026-04-27]

> **TL;DR:** Add optional support for the standalone registration-source
> module, expose fixed registration objects in the data-manager tree, and run
> registrations as cancellable background jobs with progress shown in the Jobs
> panel.

### Added
- Add an optional `BUILD_OSSIM_REGISTRATION_SOURCE` CMake path that locates and
  links `ossim-registration-source` when requested (86403cf).
- Add a Registration folder under Image Folder with right-click creation for
  Fixed registration objects and a Bundle/Floating placeholder (86403cf).
- Add a Register action for fixed registration items that queues a background
  job, mirrors writer-job behavior, and supports cancel requests (86403cf).
- Add registration job progress updates so the Jobs panel name and progress bar
  show the active floating input, auto-registration pass, and current phase
  (86403cf).
- Add handler reload plumbing to refresh source image geometries after a
  registration writes updated sidecar geometry (86403cf).

### Changed
- Register the registration-source object factory when the optional module is
  enabled so saved or created registration nodes can be rebuilt in the data
  manager (86403cf).
- Refresh registration inputs with geometry events after completion so existing
  GUI chains can pick up the new handler geometry (86403cf).

### Fixed
- Remove completed registration jobs from the Jobs panel like other background
  jobs (86403cf).
