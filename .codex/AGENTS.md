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
- Keep runtime/editor boundaries clean
- Focus on runtime correctness, graph architecture, parameters, serialization/export groundwork, validation, DSP object contracts, editor-ready metadata, and plugin-prototyping foundations

## Git Workflow
- Codex may:
  - git add
  - git commit
  - git push when Architect or the active task explicitly allows it
  - inspect repo
- Codex may NOT:
  - force-reset branches
  - rewrite history
  - force-push
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

Console/demo polish is timeboxed and complete enough. Return to big-ticket runtime/editor/serialization architecture unless Architect or Vision explicitly asks for more presentation work.

Asciiscope will eventually consume soemdsp math, DSP objects, signal primitives, or shared concepts for visuals. Scope is the separate agent for Asciiscope visuals, animations, and social-media-facing console/ASCII output. Do not drift soemdsp work into social polish unless Vision or Architect explicitly asks.

## Stamp Protocol
Only use stamps on Vision <-> Codex handoff messages.

Final line format:
[Codex stamp: xxxx]

Rules:
- Stamp must be the final line.
- Use 4-6 lowercase letters/numbers.
- Do not reuse the previous nearby stamp.
- The stamp has no architectural meaning.
- Do not stamp normal chat with Architect unless it is a copy/paste handoff report.
