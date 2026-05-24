# soemdsp Operating Instructions

## Project Identity
soemdsp is a modular DSP/runtime/reflection system evolving toward:
- realtime graph runtime
- visual modular sandbox
- plugin prototyping platform
- shader/signal experimentation platform
- scripting environment
- exportable/open modular architecture

## Development Priorities
1. Runtime correctness
2. Reflection metadata
3. Editor/layout metadata
4. Debug/inspection tooling
5. Playable sandbox
6. UI/editor integration
7. Serialization
8. Scripting/runtime export
9. Plugin/app ecosystems

## Coding Rules
- Prefer small composable commits
- Do not reformat unrelated code
- Preserve existing architecture unless explicitly instructed
- Build after bounded tasks
- Keep repo buildable
- Prefer additive changes over rewrites
- Avoid premature abstraction
- Keep debug tooling lightweight
- Prefer reflection-friendly architecture

## Git Workflow
- Codex may:
  - git add
  - git commit
  - inspect branches/status/log
- Codex should NOT:
  - git push
  - force-reset branches
  - rewrite history
unless Architect explicitly instructs it.

## Reporting Rules
Always report:
- files changed
- behavior changes
- build result
- commit hash/message
- repo status

## Architectural Direction
The project is intentionally evolving toward:
- fully inspectable runtime graphs
- editor-driven workflows
- runtime reflection
- portable/exportable DSP graphs
- modular UI systems
- future scripting integration
- future shader graph integration

Do not over-engineer.
Do not introduce heavy dependencies without approval.
Preserve momentum toward a playable modular environment.
