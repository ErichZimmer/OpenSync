# Arcane Opensync Pwa development instructions

- Use plain JavaScript, HTML, and CSS; do not introduce TypeScript or TSX.
- Keep reusable portable mechanisms in the Arcane SDK and app-specific behavior under `./`.
- Keep `./node_modules/arcane-os/runtime/arcane/css/theme.css` before app styles and import `arcane-os/modules/ThemeBootstrap.js` before app code runs.
- Use `rgb(...)` or `rgba(...)` for new CSS colors.
- Build one named app and one explicit target at a time. Native targets may be unavailable until their adapters are installed.
- Preserve complete application, model, document, message, log, diagnostic, process, and tool content. Do not truncate, clip, tail, elide, or silently discard it.
- Do not make ordinary application behavior depend on byte counts, byte limits, byte identities, hashes, digests, or byte-based admission.
- Optional hardening must remain inactive unless the user expressly selects secure: true for the exact operation. The ordinary path must remain fully functional.
- Run tests and checks only when the user explicitly selects verification or a distribution artifact requires it.
