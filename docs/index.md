# Velo documentation
This directory contains the main documentation for the Velo language, compiler pipeline,
runtime, and MVP planning.

## Language
- [Syntax](language/syntax.md)
- [Types](language/types.md)
- [Modules](language/modules.md)

## Architecture
- [Overview](architecture/overview.md)
- [Semantic analyzer](architecture/semantic.md)
- [Runtime and VM](architecture/runtime.md)
- [IR](architecture/ir.md)

## Development
- [MVP 0.1 status](development/mvp_0_1.md)
- [Adding builtin](development/adding_builtin.md)
- [Adding a language feature](development/adding_lang_features.md)
- [Diagnostics](development/diagnostics.md)
- [Example guidelines](development/example_guidelines.md)
- [MVP release checklist](development/release_checklist.md)
- [Examples](examples.md)
- [Velo 0.1.0 release notes draft](development/release_notes_v0_1.md)
- [MVP 0.2 status](development/mvp_0_2.md)

## Documentation rules
When adding a new feature:
1. update the relevant language documentation
2. update the relevant architecture documentation
3. add or update examples
4. add the feature to the MVP status file if it affects MVP scope
5. keep README concise and link to detailed docs

README should stay high-level. Detailed explanations belong in `docs/`.