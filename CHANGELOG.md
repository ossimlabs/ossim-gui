# Changelog

All notable changes to `ossim-gui` are documented here.

## [Unreleased]

> **TL;DR:** The Registration folder can now create and run Bundle/Floating
> registration jobs, default them to all-floating mode, and toggle whether
> input 0 is used as a convergence anchor. The optional registration UI now
> consumes the bridge from `ossim-autoreg`, and fixed OpenCV auto registration
> can be created, populated by drag-and-drop, validated against the tie-point
> generator factory, and reported with clearer job feedback.
> Registration creation now has a setup dialog with matcher-aware defaults, and
> fixed registration nodes use the same `Registered: <matcher>` label as the
> geometry adjustments they write. The direct Fixed registration menu and setup
> dialog now expose the same adaptive fixed-auto default, so users can choose
> the intended no-knob path without remembering which matcher currently wins.

### Added
- Add Bundle/Floating registration object creation when
  `ossim-registration-source` is enabled (b09aaf4).
- Add GeoCell registration quality report artifacts for fixed and bundle
  registration jobs, with the generated report path appended to each job
  summary when `ossim-registration-source` is enabled.
- Add a bundle-adjustment background job with progress updates, cancellation,
  geometry sidecar writes, and geometry reload propagation (b09aaf4).
- Default new Bundle/Floating registration objects to all-floating mode and add
  an item context-menu toggle for switching between all-floating and input-0
  anchored bundle adjustment (f77f01f).
- Add Fixed OpenCV Auto registration creation, direct drag-and-drop input
  wiring for registration items, registration job warning propagation, and
  tooltip reporting for factory-registered tie-point generators (7582c48).
- Add a Registration Setup dialog for fixed auto, fixed manual, bundle
  all-floating, and bundle anchored workflows with matcher-aware defaults and
  availability-checked OpenCV options (69a28cd).
- Add an explicit `Adaptive Auto (Recommended)` matcher choice to Registration
  Setup and make the direct Registration > Fixed menu create an adaptive
  fixed-auto source without requiring the setup dialog.

### Changed
- Use `ossim-autoreg`'s shared bundle defaults when
  `ossim-registration-source` is enabled, keeping GeoCell bundle setup aligned
  with the CLI default configuration model.
- Show bundle connectivity, solver backend, active parameter/image-block
  counts, normal block pairs, and residual counts in GeoCell bundle job
  summaries when `ossim-registration-source` is enabled (83090e9).
- Put the recommended Fixed Auto and Bundle Anchored registration approaches
  at the top of the Registration Setup selection list so new registrations
  start from the strongest default workflows.
- Resolve the optional registration-source bridge from `ossim-autoreg` while
  preserving the existing `ossim-registration-source` target name used by the
  GUI link path (f77f01f).
- Validate the OpenCV phase tie-point generator through
  `TiePointGeneratorFactory` and propagate optimized fixed-registration
  parameters back to connected live source geometry before writing sidecars
  (7582c48).
- Name fixed registration tree nodes as `Registered: <matcher>` and propagate
  completed fixed-registration parameters by the adjustment index reported by
  `ossim-autoreg` (69a28cd).
- Drive fixed and bundle registration setup through the shared
  `AutoRegistrationOptions` bridge from `ossim-autoreg`, preserving explicit
  matcher overrides while leaving the recommended fixed-auto path untuned so
  the shared selector can choose.

## [2026-05-03]

> **TL;DR:** Bundle/Floating registration now starts with explicit tie-point
> generation defaults so new jobs use the tested phase-correlation path with
> cubic resampling.

### Changed
- Default new Bundle/Floating registration objects to phase-correlation tie
  generation with cubic resampling (51dcdb1).

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
