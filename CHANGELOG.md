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
> Fixed registration reports now say when native affine accepted adaptive
> cleanup, saving one little expedition into the detail log.
> GeoCell also has a one-click native-affine bundle path now, so fixed and
> bundle experiments can be compared without hand-tuning the setup dialog.
> Bundle quality reports now show per-pair execution paths too, which makes
> native bundle fallback behavior visible without guessing.
> Sparse bundle-edge support now appears in GeoCell advisories and reports,
> so a connected bundle graph cannot hide behind a tiny RMSE alone.
> Bundle quality reports now include acceptance tiers and compact per-pair
> route summaries for faster visual swipe triage.
> GeoCell bundle setup now exposes the shared native bundle neighbor-span
> policy, and bundle quality reports record whether a run used all pairs or a
> bounded neighbor graph.
> GeoCell now has an explicit native-affine strip bundle preset, so ordered
> strip datasets can start from the tested `neighbor_span_1` graph without
> hand-editing the numeric source property.
> Bundle reports now include the GeoCell launch preset, and the setup dialog
> has an `Auto` pair-policy placeholder that currently resolves conservatively
> to all pairs.
> That GeoCell `Auto` pair-policy choice now flows through the shared
> `ossim-autoreg` source options instead of staying as a local GUI-only label.
> GeoCell bundle reports now show the shared Auto pair-policy diagnostics and
> advisory strip-candidate evidence used by CLI/source runs.
> GeoCell bundle reports now headline the resolved pair graph, so promoted Auto
> strip runs show `neighbor_span_1` instead of the requested placeholder.
> GeoCell bundle completion text now names the resolved Auto pair policy and
> calls out all-pairs fallback recovery when it happens.
> GeoCell bundle quality reports now include explicit Auto pair-policy fallback
> attempted, reason, and result fields.
> GeoCell bundle completion text now includes compact Auto pair-policy speed
> evidence, including generated/full pairs, saved pairs, timing, and fallback
> result when present.
> The native-affine strip bundle action now requests the shared Auto pair
> policy, so clear strips still promote to `neighbor_span_1` while ambiguous
> cases can fall back to all pairs without changing the menu workflow.
> Native-affine bundle menu labels now present the choice as General versus
> Ordered Strip, keeping advanced pair-policy details in setup/properties.
> GeoCell bundle reports now include the same native matcher policy diagnostic
> as CLI runs, so visual swipe testing can see when native-affine stood alone
> or needed recovery.
> GeoCell now uses the shared native matcher policy helper and reports the
> matcher Auto would choose before the UI starts applying that decision.
> GeoCell bundle completion text now surfaces the native matcher would-choose
> result, while bundle reports include the shared native matcher policy mode and
> reason fields.
> GeoCell bundle reports now state whether explicit native matcher Auto policy
> applied and what action it took.
> Registration and chain context menus now use the same Auto action names, the
> ordered strip preset really uses adjacent strip pairs, and bundle reports
> show both the configured pair policy and strip-edge support.
> GeoCell shutdown now cancels and detaches registration job callbacks before
> the data manager widget disappears, so closing during a long registration
> should be much less exciting. Adjustable parameters can now be locked and
> unlocked directly from GeoCell, so that persistent flag no longer requires
> a side quest outside the dialog.
> Bundle reports now show the same per-edge value and guarded weakest-edge
> refinement evidence as the CLI, without teaching GeoCell any algorithms.
> They now include the full factory-priority candidate order and shared
> selection reason too, so plugin participation is visible without GUI policy.
> Completed registration objects now retain their quality reports for inline
> review or a larger report window, and context menus no longer activate an
> action merely because the initiating mouse button was released over it.
> Registration Setup now lives in a focused optional component and obtains
> bundle solver choices from the autoregistration factory while carrying
> explicit pair and anchor selections through the shared source API.
> Fixed and bundle registration jobs now live in focused modules and launch
> registered sources through shared setup profiles, leaving the data manager
> to manage the interface instead of quietly moonlighting as an engine.
> Completed registration reports are now easier to reopen, read, wrap, and
> copy from either the registration object or its report child.
> GeoCell now discovers tailored editors and quick-registration workflows
> through factories, while selected-image registration has guided preflight,
> stable sizing, explicit run intent, concise results, and built-in swipe
> verification without teaching the GUI concrete algorithms.
> Image interaction modes now use the same extension-friendly pattern, while
> Standard Navigation stays quietly in charge until another applicable mode
> is actually registered.
> Completed fixed and bundle registrations now have a reusable tie-point
> inspection workbench with scalable markers, edge selection, synchronized
> swipe views, and predictable window reuse. Geometry adjustments also gain a
> visible Lock-header checkbox for locking or unlocking all parameters at once.
> Registration preflight now calls out images with no overlap and can exclude
> them from a run without removing them from the Data Manager.
> It now also warns when workflow pairing cannot reach a control or anchor,
> detects disconnected all-floating graphs, and supports multi-row exclusion.
> New standard mosaics now put fine-resolution imagery above coarser
> backgrounds, and selected combiners can register their direct inputs while
> the existing mosaic view reflects accepted geometry updates. Combiner
> registration now offers the same factory-backed quick choices as selecting
> the input chains directly, so the workflow does not change with viewpoint.
> Reordering an existing combiner input now preserves its live connections,
> including view propagation and zoom, and dropping above the first row moves
> the selected layer to input zero without a duplicate-layer warning.

### Fixed
- Reorder a single existing combiner input through OSSIM's in-place connection
  movement APIs instead of disconnecting and rebuilding the full input list,
  preserving downstream view propagation and allowing a direct move to the
  first input.
- Open GeoCell context menus through Qt's context-menu event so the mouse
  release that requested the menu cannot immediately activate the action under
  the pointer (`8d29413`).
- Prevent GeoCell shutdown crashes when a registration, image-open, or staging
  job is still running by detaching widget callbacks, canceling queued work,
  draining pending widget events, and waiting for the worker queue from the
  main window close path (`06576ee`).
- Make fixed and bundle registration cancellation observe the shared widget
  shutdown request directly, clear source callbacks with exception-safe scopes,
  and skip result/report processing once close cancellation has completed
  (`37b931e`).

### Added
- Give selected combiners the same factory-driven Quick Registration choices
  as multi-chain selections (`cb6c1fd`).
  - Pass the combiner's direct image inputs through the shared registered-preset
    menu and existing launch dialog.
  - Preserve explicit Run intent, input roles, pairing, and parallel settings
    instead of introducing a separate combiner-only execution path.
- Add a combiner-centered registration workflow and resolution-aware mosaic
  creation defaults (`c056d00`).
  - Register a selected combiner's direct image inputs through the existing
    guided, factory-backed registration setup.
  - Keep the existing mosaic display connected so accepted fixed-registration
    geometry updates refresh the composed view.
  - Order new A-over-B, Blend, and Feather mosaics from fine foreground imagery
    to coarse background imagery while preserving selection order when native
    ground resolution is unavailable.
  - Leave existing combiners, factory combiners, and semantic-role combiners
    unchanged.
- Add workflow-aware registration pairing preflight (`dc477ed`).
  - Warn when fixed floating inputs have no fixed/control overlap.
  - Trace anchored overlap paths and detect disconnected all-floating graphs
    under the selected bundle pair policy.
  - Allow Command/Control and Shift multi-row exclusion while remapping anchors
    and explicit pairs.
- Make non-overlapping registration inputs actionable in the selected-image
  preflight (`6ffa75f`).
  - Mark an input as `No overlap` only when geometry overlap is available for
    every selected peer and every overlap ratio is zero.
  - Summarize isolated inputs and provide a confirmed one-click exclusion that
    leaves the original images in the Data Manager.
  - Recalculate launch eligibility and remap bundle anchors and explicit pairs
    after exclusions.
- Add reusable fixed and bundle tie-point inspection to completed registration
  results (`9106fa3`, `9c0655e`).
  - Render fixed/moving observations, applied corrections, removed ties, and
    scalable selection markers in a dedicated always-on-top workbench.
  - Retain completed fixed snapshots and bundle pair evidence so inspection
    does not rerun registration.
  - Select bundle edges and tie rows while keeping the shared swipe display
    centered and synchronized across zoom and partial-overlap inputs.
  - Reuse one swipe display per registration from both `Swipe` and
    `Inspect Ties`, recreating it only after the retained display closes.
  - Use `ossimDrect` intersection and union bounds for the selected swipe pair
    while preserving the established swipe compositing behavior.
  - Add a tri-state checkbox to the `Lock` column heading for locking or
    unlocking every parameter in the current geometry adjustment.
  - Manually verify fixed and all-image Celtic bundle inspection, edge
    selection, zoom behavior, window reuse, and parameter locking.
- Add a generic object-manipulator registry for image-view interaction modes
  (`750f876`).
  - Register existing Standard Navigation as the default provider.
  - Discover, prioritize, create, replace, and unregister manipulators by
    object type or applicability predicate.
  - Expose named manipulator replacement through `ImageScrollView`.
  - Show an interaction-mode chooser only when more than one provider applies,
    preserving the current single-mode toolbar and navigation behavior.
- Add a generic tailored-editor registry and a factory-driven selected-image
  registration workflow (`7ce314e`).
  - Keep `Properties...` as the generic fallback while existing image tools and
    Registration Setup register as tailored editors.
  - Build Quick Registration choices from public autoregistration descriptors
    instead of GUI-owned matcher menus.
  - Preflight selected input order, entries, geometry mobility, overlap, roles,
    anchors, and explicit pairs before setup creation or execution.
  - Separate `Create Setup` from `Run Registration`, with execution disabled
    when the selected inputs cannot run the requested workflow.
  - Present completion status, summary, advisory, rerun, full-report, and
    multi-layer swipe actions in the registration object.
  - Keep Basic and Advanced fields in stable tabs, improve field and path
    sizing, and give the Data Manager a responsive initial width.
  - Manually verify fixed/floating native-affine NCC registration, geometry
    persistence, live adjustment, result reporting, and swipe inspection.
- Make completed registration reports directly accessible from the
  registration object and report child through context menus and double-click,
  and add copy-all, line-wrap, and resizable full-report controls. Verified
  with a successful Celtic all-registration run covering 7 floating inputs,
  482 retained tie points, and 7 in-place live geometry updates (`deb6756`).
- Retain completed fixed and bundle quality reports on their GeoCell
  registration objects, including the generated report path, an inline preview,
  and a resizable full-report window (`8d29413`).
- Show the shared ordered weak-edge matcher candidates and factory selection
  reason in GeoCell bundle quality reports (`5e023ba`).
- Show shared bundle pair residual-value and bounded weakest-edge refinement
  diagnostics in GeoCell quality reports (`9a1a57c`).
- Add a `Lock` checkbox column to the Adjustable Parameter dialog so GeoCell
  users can toggle each parameter lock flag and persist the change through the
  existing OSSIM adjustment interface (`708edb2`).
- Show shared strip-edge quality advisories in GeoCell bundle summaries and
  reports, including sparse adjacent edge counts and the weakest adjacent edge
  tie support (`2f1b644`).
- Add configured bundle pair policy and neighbor-span fields to GeoCell bundle
  reports so the selected preset is visible beside the resolved graph
  diagnostics (`c9cfc94`).
- Add generated/full pair counts, saved pair count, pair-generation timing, and
  fallback result to GeoCell's compact bundle pair-policy completion summary.
- Add explicit Auto bundle pair-policy fallback fields to GeoCell quality
  reports, matching the shared `ossim-autoreg` diagnostics.
- Show the resolved Auto bundle pair-policy decision in GeoCell completion
  summaries and include all-pairs fallback recovery in advisory text.
- Report the resolved bundle pair graph in GeoCell bundle quality reports when
  shared Auto policy diagnostics promote a run to a bounded strip graph.
- Show shared bundle pair-policy diagnostics in GeoCell bundle quality reports,
  including requested/resolved policy, resolution reason, and advisory
  strip-candidate overlap evidence.
- Flow GeoCell's bundle pair policy selector through shared
  `ossim-autoreg` bundle pair policy options, keeping `Auto` reportable as
  `auto_all_pairs` while it remains conservative.
- Add `bundle_launch_preset` to GeoCell bundle quality reports and tag
  default, setup-dialog, native-affine, and native-affine strip bundle sources
  with stable launch labels.
- Add an `Auto` option to the GeoCell bundle pair policy selector, currently
  resolving to the conservative all-pairs policy until strip detection is
  taught to pick a bounded graph.
- Add `Bundle Native Affine Strip Auto` actions for Registration-folder and
  selected image-chain context menus, using the shared `neighbor_span_1`
  bundle pair policy.
- Replace the raw GeoCell bundle neighbor-span setup knob with an explicit
  `Bundle pair policy` selector plus enabled neighbor-span value field.
- Wire GeoCell's selected bundle pair policy into bundle registration sources,
  created-item tooltips, and generated bundle quality reports as
  `bundle_pair_policy`.
- Add bundle acceptance tiers and compact per-pair route summaries to GeoCell
  bundle registration quality reports.
- Surface sparse bundle-edge support issues in GeoCell bundle advisories and
  quality reports.
- Include bundle pair execution paths in GeoCell bundle-registration quality
  reports so native-affine edges and accepted matcher fallbacks are visible.
- Add `Bundle Native Affine Auto` actions for Registration-folder and selected
  image-chain context menus, creating an all-floating bundle source with the
  shared native-affine setup defaults.
- Include fixed-auto execution paths in GeoCell fixed-registration quality
  reports, and mention native cleanup in the completion summary only for
  inputs where cleanup replaced the direct native-affine result.
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
- Extract fixed and bundle execution, cancellation, progress, runtime context,
  and report handling from `DataManagerWidget`; create sources and apply named
  setup profiles through the shared factory-driven autoregistration APIs
  (`2b27f0b`).
- Make `Bundle Native Affine Strip Auto` request explicit adjacent strip pairs
  and update its tooltip so ordered flightline runs use the graph promised by
  the menu label (`4770ff6`).
- Align GeoCell Registration-folder and selected chain/source context-menu Auto
  labels and creation paths, including adding the missing `Fixed OpenCV Auto`
  selected-chain action (`47ef0b1`).
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
- Extract Registration Setup from the data manager, populate bundle solvers
  from factory descriptors, and pass explicit pair, anchor, and stable solver
  type selections through the shared optional registration bridge (`b3a2e30`).

## [2026-07-19]

> **TL;DR:** Registration Setup now discovers available matchers and their
> recommended settings from `ossim-autoreg`, so GeoCell no longer maintains a
> private list of algorithms or OpenCV-specific defaults. Bundle reports now
> explain each ordered factory-candidate attempt without moving algorithm
> policy into the GUI.

### Changed
- Populate Registration Setup from autoregistration factory descriptors and
  apply factory-owned recommendations for explicit matcher choices (`dfd647a`).
- Report the shared candidate limit and per-attempt matcher, decision, tie, and
  RMS evidence for bounded bundle edge fallback (`e3892ab`).

## [2026-07-18]

> **TL;DR:** OSSIM GUI builds now continue without registration support when
> the optional `ossim-autoreg` bridge is unavailable. The parent OSSIM build
> now owns whether that bridge is included, leaving GUI to consume it when
> present instead of running a second round of package discovery.

### Fixed
- Keep `ossim-registration-source` optional by warning and disabling its GUI
  integration when the imported bridge target cannot be found, and avoid stale
  CMake package-registry entries during discovery (`066e56d`).

### Changed
- Remove GUI-owned registration build controls and package discovery, consume
  the bridge target supplied by the parent build, and use
  `OSSIM_AUTOREGISTRATION_ENABLED` for conditional GUI code (`a570ccf`).

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
