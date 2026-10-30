# Preserved native analysis and historical source exposure

This is a release-content finding in the final combined milestone of the
[single scope ledger](MAP_AUDIT_FOLLOWUPS_20261028.md#current-milestone-and-scope-ledger).
It does not reopen any Closed gameplay subsystem. No game was launched or
controlled, no history was rewritten and nothing was uploaded.

## Current cleanup

The final content inspection found 200 recovered game-analysis files inside
the checkout, including ignored files. They contain disassembly, recovered
declarations/bodies or original script exports. Twelve OpenAL example files
were explicitly excluded: those are legitimate attributed vendor source.
Original mod implementations, bakes, factual manifests, offsets, locators and
behavioral evidence were preserved.

Every affected original was copied with its relative path and verified SHA-256
to `D:/Games/Dev/Unity/FFPR/tools/repository_sources_20261029/`, outside every
Git checkout. Thirty tracked disassembly files under docs/re are now plain
source-free references to their preserved originals. The other 170 local raw
files were removed only after verification. Companion factual fresh-extraction
inputs were also copied so the old comparison can still consume its complete
external dataset. The factual preservation manifest is
`build_check/map_audit_final_20261029/source_cleanup.json`; a second manifest
lives in the external archive. No backup of recovered bodies was left in Git.

Fourteen historical exporters now use the checked external directory selected
by `tools/re/evidence_paths.py`, under
`D:/Games/Dev/Unity/FFPR/tools/native_evidence/`. It rejects path escape and any
Git ancestor before creating an output directory. Factual metadata can still
be written to docs/re. The historical fresh-bake comparison now consumes its
preserved external source dataset. Modern audit exporters already use external
analysis directories. Syntax checks do not execute the game or the exporters.

## History and publication boundary

All thirty tracked files are present in the pre-cleanup HEAD tree b6cdbd34,
in this shared native repository, not just the older six managed mods. A
follow-up verification found53757 native instruction lines; every preserved
original matches its historical blob after Git's CRLF/LF normalization.
Representative source-file history includes8697371,3bacb53 and9199500 from
October12. Exact paths, historical hashes, archive hashes and counts are in
build_check/map_audit_final_20261029/history_location_confirmation.json.
The repository is the existing FFPRCore-Screen-Reader checkout and its same-named
public remote. Remote history, old tags, releases and forks have not been
comprehensively inspected by this local check.

Human clarification, October29: verify the shared mod is deployed to all six
games and commit all current changes. Ordinary commits of the cleaned current
tree proceed. Historical cleanup will be decided when the user is at the PC;
the options include a history rewrite or replacement repository. The earlier
blanket publication/commit hold was an overly broad agent decision, not an
additional user approval requirement. Do not claim that old history is clean.

The safe next history operation is a separately authorized migration: inventory
reachable commits/tags and remote refs for the affected paths and other source
exports; preserve a complete local recovery copy outside every checkout; prepare
a clean mod-only history; verify retained mod work, attribution, all refs and
package contents; then obtain approval for the exact shared-history replacement
and communicate its effect on existing clones. No reset, force push, tag removal,
repository replacement or deletion is authorized by this audit itself.

This finding is a deferred historical-content issue. Current commits and local
delivery of the checked compiled mod continue under the user's authorization.
All six installed payloads were reverified against the same132 scanned files
during the commit follow-up. Exact native/frame/audio performance and live
gameplay remain separate evidence. No destructive history action is performed.
