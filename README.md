# sentence-reader

Sentence Reader is an English sentence parser written in C++. It now includes a local website that sends sentences to the C++ parser and renders each interpretation in the browser.

https://sentence-reader.up.railway.app/

## What changed

- The parser now supports `--json` output for machine-readable integrations.
- A zero-dependency Node web server exposes the parser at `POST /api/parse`.
- A browser UI in `web/public/` lets you enter sentences, try built-in examples, and inspect each parse as a syntax tree.
- Input normalization, unknown-word reporting, and inflected verb handling from the recent parser cleanup are preserved in both the CLI and the website.

## Run the parser

```bash
cmake -S main -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Human-readable mode:

```bash
./build/sentence-reader --sentence "I saw the man with the telescope"
```

JSON mode:

```bash
./build/sentence-reader --json --sentence "I saw the man with the telescope"
```

Interactive mode:

```bash
./build/sentence-reader
```

## Run the website

Build the parser once, then start the local server:

```bash
npm run build:parser
npm start
```

Open [http://127.0.0.1:4173](http://127.0.0.1:4173).

For a public server deployment, the easiest path is Docker Compose:

```bash
docker compose up --build -d
```

Deployment notes and copy-ready server examples live in `DEPLOY.md`.

## Website architecture

- `main/src/main.cpp`: CLI entry point with human-readable and JSON output modes
- `web/server.mjs`: local HTTP server and parser bridge
- `web/public/index.html`: site structure
- `web/public/styles.css`: minimal UI styling and tree layout
- `web/public/app.js`: client-side parsing flow and syntax tree rendering
- `deploy/Caddyfile.example`: reverse proxy template for a public domain
- `deploy/sentence-reader.service`: optional `systemd` unit for native Linux hosting

## Example

```bash
curl -s -X POST http://127.0.0.1:4173/api/parse \
  -H 'Content-Type: application/json' \
  -d '{"sentence":"I saw the man with the telescope."}'
```
