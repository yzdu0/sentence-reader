# Deploying Sentence Reader

This project is easiest to deploy in Docker.

## Option 1: Docker Compose

Requirements:

- Docker
- Docker Compose

From the project root:

```bash
docker compose up --build -d
```

The site will be available on port `4173`.

Health check:

```bash
curl http://127.0.0.1:4173/api/health
```

Stop it:

```bash
docker compose down
```

Update it later:

```bash
git pull
docker compose up --build -d
```

## Option 2: Native Linux server

Requirements:

- Node.js 22+
- CMake
- A C++ compiler

Build and run:

```bash
npm run build:parser
npm run start:prod
```

For a background process on a real server, use a supervisor such as `systemd`, `pm2`, or Docker. A ready-made `systemd` unit file is included at `deploy/sentence-reader.service`.

## Reverse proxy and HTTPS

For a public deployment, put Nginx or Caddy in front of the app and proxy traffic to `127.0.0.1:4173`.

Example Caddy config:

```caddy
your-domain.example {
  reverse_proxy 127.0.0.1:4173
}
```

The same config is included as `deploy/Caddyfile.example`. With Caddy, once your domain points at the server, HTTPS is handled automatically.

## Railway request logs

If you want to record every sentence submitted to `POST /api/parse`, set this environment variable on Railway:

```bash
LOG_SENTENCE_REQUESTS=true
```

When enabled, the app writes one JSON log line per sentence to standard output, which Railway captures in its deploy/runtime logs. Example fields include:

- `sentence`
- `normalizedSentence`
- `status`
- `success`
- `interpretationCount`
- `unknownWords`
- `clientIp`
- `userAgent`

This is the best fit for Railway because container files are not durable across restarts or redeploys. If you need long-term retention or analytics, forward these logs into a database or external log sink.

## Easiest real-server setup

If you want the shortest path on a VPS:

1. Install Docker and Docker Compose on the server.
2. Clone this repo onto the server.
3. Run `docker compose up --build -d`.
4. Confirm it with `curl http://127.0.0.1:4173/api/health`.
5. Put Caddy in front of it using `deploy/Caddyfile.example`.

That route avoids having to install Node, CMake, or a compiler directly on the host.

## Notes

- The parser binary is auto-discovered in `build/`.
- You can override the binary path with `PARSER_BINARY=/absolute/path/to/sentence-reader`.
- The reference page reads from `main/src/Grammar.cpp` and `main/data/vocab.txt`, so those files are included in the Docker image too.
