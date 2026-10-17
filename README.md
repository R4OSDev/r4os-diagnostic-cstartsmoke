# CSTARTD.R4X

`CSTARTD.R4X` is an independent R4OS diagnostic program implemented in C.

## Package

- Version: `0.1.3`
- Image target: `/R4OS/SOFTWARE/TERMINAL/DIAG/CSTARTD.R4X`
- Image scope: `test`
- Canonical project manifest: `module.R4MF`

The manifest is the single source of truth for the artifact, imports, image
target, and package metadata.

## Build

On Windows:

    Build.bat

On Linux or macOS:

    ./Build.sh

The build starters resolve the current local R4OS dependency checkouts through
`Settings.R4S`. The URL and hash entries in `build.zig.zon` record the
last verified standalone dependency identities; workspace builds use the
mapped local checkouts.

## Documentation

Detailed German technical notes from the migration are preserved in
`DOCUMENTATION.de.txt`. Source-transfer provenance is recorded in
`PROVENANCE.txt`.

## License

Original R4OS material is licensed under Apache License 2.0. See `LICENSE`
and `NOTICE`. Any repository-specific external material is documented in
`THIRD_PARTY_NOTICES.md`.

`/QUIET` runs the same C entry, ABI, import and VM checks, omitting successful
progress output. Errors remain visible. CleanupDiag uses this mode during
exact memory measurements because the parent intentionally retains child
output after reaping. Normal invocation retains all progress messages.

`/STACKPROBE` explicitly checks one 384-KB local frame with 8-KB alignment
and volatile page writes/readback. Success prints `CSTARTD large frame: OK`.
It exercises the freestanding C inline probes and demand stack growth;
normal invocation and `/QUIET` keep their existing ABI/import/VM checks.
