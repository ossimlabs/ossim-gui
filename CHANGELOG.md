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
> The adjustable-parameter editor can now explicitly rebuild adjustment 0 from
> the model defaults, which makes revised sensor-model sigmas easier to reload.
> Fixed registration jobs now surface low-control texture warnings from
> `ossim-autoreg` in both the completion dialog and generated quality report.
> Bundle registration jobs now surface source-level advisory and matcher
> recovery details in the same GeoCell feedback path. Registration property
> editing now uses the generic OSSIM property view for post-creation tuning,
> including dropdowns for enumerated choices and less fussy checkbox/combobox
> editor behavior. GeoCell fixed-auto registration now keeps the same shared
> coarse-to-fine defaults as the CLI/source path, so default fixed runs do not
> quietly stop early. Registration reports now include the OSSIM runtime and
> elevation context so GeoCell swipe reviews can confirm whether they used the
> same preferences and terrain sources as the CLI. Fixed-auto launch reports now
> snapshot the settings that were active when the job started, even if the user
> edits properties while the registration is still running. Long fixed-auto
> jobs now label live preview, accepted, restored, and coarse geometry updates
> so GeoCell progress reads like the registration is actually moving, and the
> latest RMSE-bearing geometry update stays visible while generic pass messages
> continue. GeoCell can now opt into tie-point timing diagnostics only when
> profiling is needed.
> GeoCell setup can now tune the OpenCV RANSAC prefilter and threshold through
> the same shared registration options used by CLI/source runs.
> GeoCell setup can now tune the native-affine NCC score-margin gate too, so
> GUI runs can use the same ambiguity filter as CLI experiments.
> GeoCell can now launch the tuned native-affine fixed-auto preset directly
> from registration menus and selected image chains.

### Added
- Add GeoCell `Fixed Native Affine Auto` actions for Registration-folder and
  selected image-chain context menus, and hydrate native-affine setup defaults
  from the shared `fixed:native-affine` preset so GUI runs match the
  CLI/source path (`8eb2fd7`).
- Add a GeoCell Registration Setup control for the native-affine minimum score
  margin, wiring it into fixed and bundle registration sources plus created
  item tooltips so GUI runs expose the same ambiguity filter as CLI/source
  registration (504582a).
- Add GeoCell Registration Setup controls for the OpenCV RANSAC prefilter and
  threshold, wiring them into fixed and bundle registration sources and the
  created item tooltip so GUI speed experiments expose the same coherence gate
  as CLI runs (0fad5f5).
- Add a GeoCell `Tie timing diagnostics` setup control that maps to the shared
  `ossim-autoreg` timing knob and explains it is for profiling rather than
  normal registration (86a6637).
- Include fixed-registration launch input/status and settings snapshots in
  GeoCell quality reports so long-running jobs can be audited against the
  values they actually used.
- Include OSSIM runtime context in generated fixed and bundle registration
  quality reports, including `OSSIM_PREFS_FILE`, `OSSIM_DATA`, plugin path,
  elevation database connections, and currently opened elevation cells.
- Include fixed-registration source detail logs in generated GeoCell
  quality reports so adaptive candidate selection and final-quality mode
  receipts are visible from GUI runs (0472469).
- Add GeoCell bundle quality-report fields for search-span recovery, minimum
  bundle-edge tie support, sparse-edge counts, and bundle edge support advisory
  status (c6d5f41).
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
- Add fixed-registration effective target/search span and fixed scene/search
  span policy fields to GeoCell generated quality reports when
  `ossim-registration-source` is enabled.
- Add fixed-registration quality advisories to GeoCell generated quality
  reports and show successful registration warning dialogs when
  `ossim-autoreg` flags accepted low-control texture-limited geometry.
- Add bundle-registration advisory dialogs and quality-report fields for
  bound-pressure advisories and guarded matcher recovery when
  `ossim-registration-source` is enabled.
- Add Source/Chain popup registration actions so selected image items can
  launch the default Fixed or Bundle All-Floating registration directly,
  including automatic input connection and job start.
- Add a Model Defaults button to the adjustable-parameter editor that clears
  all adjustment history, calls the model's `initAdjustableParameters()`, and
  reloads a fresh adjustment 0 from the current model defaults.
- Add GUI-thread synchronized fixed-registration progress application so
  accepted GeoCell geometry updates refresh the live image chain as each
  floating input completes without stacking duplicate adjustment slots.
- Add GUI-thread synchronized bundle geometry saves so bundle registration
  sidecar writes and reload propagation use the same main-thread path as fixed
  registration.
- Clarify GeoCell bundle result summaries and quality reports with the active
  bundle mode, motion policy, target RMSE, pass count, and all-floating datum
  prior so anchored and all-floating solves are easier to compare.
- Add GeoCell registration setup controls for fixed/floating parallel input
  jobs and adaptive candidate-bank threads, matching the CLI threading knobs
  exposed by `ossim-autoreg`.
- Add generic property-editor support for registration-source settings after
  creation, including dropdown commits for constrained string properties and
  checkbox/combobox editors that no longer fight the table paint layer.

### Changed
- Keep the latest fixed-registration geometry progress label visible through
  generic tie-generation updates so RMSE/tie-count feedback remains readable
  during long GeoCell runs.
- Label fixed-registration job progress as preview, accepted, restored, or
  coarse geometry when live pass updates arrive from `ossim-autoreg`.
- Create GeoCell default Fixed Auto registrations from the shared adaptive
  fixed-auto defaults instead of a preferred concrete matcher, keeping the
  source bridge aligned with the CLI/source API path.
- Show bundle search-span recovery messages alongside bound-pressure advisories
  so GeoCell completion feedback matches the CLI/source diagnostics (c6d5f41).
- Use `ossim-autoreg`'s shared bundle defaults when
  `ossim-registration-source` is enabled, keeping GeoCell bundle setup aligned
  with the CLI default configuration model.
- Show bundle connectivity, solver backend, active parameter/image-block
  counts, normal block pairs, and residual counts in GeoCell bundle job
  summaries when `ossim-registration-source` is enabled (83090e9).
- Show shared dense-alternate bundle tie diagnostics in GeoCell bundle job
  summaries and generated quality reports, including acceptance reason,
  spread, translation-consistency, and score evidence.
- Show shared bundle normal-equation block-pair capacity and density in GeoCell
  quality reports so sparse/Schur solver readiness is visible from GUI runs.
- Show the shared two-image bundle model-freedom advisory in
  GeoCell bundle quality reports when `ossim-registration-source` is enabled.
- Show the shared guarded bundle edge-prune policy in GeoCell bundle quality
  reports so rejected prune/re-solve proofs are visible from GUI runs.
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
- Make deleting the final adjustable-parameter entry use the same model-default
  reload path as the new button instead of leaving stale adjustment state in
  the editor.

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
- Keep GeoCell fixed-auto registration on the shared CLI/source default pass
  schedule instead of forcing the old four-pass setup, restoring the expected
  geometry update for the Celtic `001` fixed / `002` floating workflow
  (88ab3c2).
- Remove completed registration jobs from the Jobs panel like other background
  jobs (86403cf).
