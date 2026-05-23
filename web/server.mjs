import { execFile } from "node:child_process";
import { existsSync } from "node:fs";
import { readFile } from "node:fs/promises";
import { createServer } from "node:http";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { promisify } from "node:util";

const execFileAsync = promisify(execFile);
const serverDir = path.dirname(fileURLToPath(import.meta.url));
const rootDir = path.resolve(serverDir, "..");
const publicDir = path.join(serverDir, "public");
const host = process.env.HOST || "127.0.0.1";
const port = Number(process.env.PORT || "4173");
const explicitParserBinary = process.env.PARSER_BINARY || "";
const logSentenceRequests = /^(1|true|yes|on)$/i.test(process.env.LOG_SENTENCE_REQUESTS || "");
const maxRequestBytes = 32 * 1024;

const mimeTypes = {
  ".css": "text/css; charset=utf-8",
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".svg": "image/svg+xml"
};

function resolveParserBinary() {
  if (explicitParserBinary && existsSync(explicitParserBinary)) {
    return explicitParserBinary;
  }

  const candidates = [
    path.join(rootDir, "build", "sentence-reader"),
    path.join(rootDir, "build", "Debug", "sentence-reader"),
    path.join(rootDir, "build", "Release", "sentence-reader")
  ];

  return candidates.find((candidate) => existsSync(candidate)) || null;
}

function stripCppComments(source) {
  return source
    .replace(/\/\*[\s\S]*?\*\//g, "")
    .replace(/\/\/.*$/gm, "");
}

async function loadGrammarRules() {
  const grammarPath = path.join(rootDir, "main", "src", "Grammar.cpp");
  const source = await readFile(grammarPath, "utf8");
  const stripped = stripCppComments(source);
  const match = stripped.match(/rules_string\s*=\s*\{([\s\S]*?)\};/);

  if (!match) {
    throw new Error("Could not extract grammar rules from Grammar.cpp.");
  }

  return [...match[1].matchAll(/"([^"]+)"/g)].map((entry) => entry[1].trim());
}

async function loadVocabularyGroups() {
  const vocabPath = path.join(rootDir, "main", "data", "vocab.txt");
  const source = await readFile(vocabPath, "utf8");
  const groups = new Map();

  for (const rawLine of source.split(/\r?\n/)) {
    const line = rawLine.trim();
    if (!line || line.startsWith("#")) {
      continue;
    }

    const colonIndex = line.indexOf(":");
    if (colonIndex === -1) {
      continue;
    }

    const tag = line.slice(0, colonIndex).trim();
    const value = line.slice(colonIndex + 1).trim();

    if (!groups.has(tag)) {
      groups.set(tag, []);
    }

    if (value.includes("|") || ["V", "N", "Adj"].includes(tag)) {
      groups.get(tag).push(value);
      continue;
    }

    for (const token of value.split(/\s+/)) {
      if (token) {
        groups.get(tag).push(token);
      }
    }
  }

  return [...groups.entries()]
    .map(([tag, entries]) => ({ tag, entries }))
    .sort((left, right) => left.tag.localeCompare(right.tag));
}

function sendJson(response, statusCode, payload) {
  response.writeHead(statusCode, {
    "Cache-Control": "no-store",
    "Content-Type": "application/json; charset=utf-8"
  });
  response.end(JSON.stringify(payload));
}

function sendText(response, statusCode, message) {
  response.writeHead(statusCode, {
    "Cache-Control": "no-store",
    "Content-Type": "text/plain; charset=utf-8"
  });
  response.end(message);
}

async function readJsonBody(request) {
  return await new Promise((resolve, reject) => {
    let body = "";
    let size = 0;

    request.setEncoding("utf8");

    request.on("data", (chunk) => {
      size += chunk.length;
      if (size > maxRequestBytes) {
        reject(new Error("Request body too large."));
        request.destroy();
        return;
      }

      body += chunk;
    });

    request.on("end", () => {
      try {
        resolve(body ? JSON.parse(body) : {});
      } catch {
        reject(new Error("Request body must be valid JSON."));
      }
    });

    request.on("error", reject);
  });
}

function getHeaderValue(value) {
  if (typeof value === "string") {
    return value;
  }

  if (Array.isArray(value) && value.length > 0) {
    return value[0];
  }

  return "";
}

function getClientIp(request) {
  const forwardedFor = getHeaderValue(request.headers["x-forwarded-for"]);
  if (forwardedFor) {
    return forwardedFor.split(",")[0].trim();
  }

  const realIp = getHeaderValue(request.headers["x-real-ip"]);
  if (realIp) {
    return realIp.trim();
  }

  return request.socket?.remoteAddress || "";
}

function logSentenceRequest(request, details) {
  if (!logSentenceRequests || !details.sentence) {
    return;
  }

  console.log(JSON.stringify({
    event: "parse_request",
    timestamp: new Date().toISOString(),
    clientIp: getClientIp(request),
    userAgent: getHeaderValue(request.headers["user-agent"]),
    ...details
  }));
}

async function runParser(sentence) {
  const parserBinary = resolveParserBinary();
  if (!parserBinary) {
    throw new Error("Parser binary not found. Run `npm run build:parser` first.");
  }

  try {
    const { stdout } = await execFileAsync(
      parserBinary,
      ["--json", "--sentence", sentence],
      {
        cwd: rootDir,
        maxBuffer: 2 * 1024 * 1024
      }
    );

    return JSON.parse(stdout);
  } catch (error) {
    if (typeof error.stdout === "string" && error.stdout.trim()) {
      try {
        return JSON.parse(error.stdout);
      } catch {
      }
    }

    throw new Error(error.stderr?.trim() || error.message || "The parser failed to run.");
  }
}

async function serveStaticAsset(requestPath, response) {
  const normalizedPath = requestPath === "/" ? "/index.html" : requestPath;
  const absolutePath = path.normalize(path.join(publicDir, normalizedPath));

  if (!absolutePath.startsWith(publicDir)) {
    sendText(response, 403, "Forbidden");
    return;
  }

  try {
    const file = await readFile(absolutePath);
    const extension = path.extname(absolutePath);
    response.writeHead(200, {
      "Content-Type": mimeTypes[extension] || "application/octet-stream"
    });
    response.end(file);
  } catch {
    sendText(response, 404, "Not found");
  }
}

const server = createServer(async (request, response) => {
  const url = new URL(request.url || "/", `http://${request.headers.host || `${host}:${port}`}`);

  if (request.method === "GET" && url.pathname === "/api/health") {
    sendJson(response, 200, {
      ok: true,
      parserBinary: resolveParserBinary() || ""
    });
    return;
  }

  if (request.method === "GET" && url.pathname === "/api/reference") {
    try {
      const [grammarRules, vocabularyGroups] = await Promise.all([
        loadGrammarRules(),
        loadVocabularyGroups()
      ]);

      sendJson(response, 200, {
        grammarRules,
        vocabularyGroups
      });
    } catch (error) {
      sendJson(response, 500, {
        error: error.message || "Could not load reference data."
      });
    }
    return;
  }

  if (request.method === "POST" && url.pathname === "/api/parse") {
    let sentence = "";

    try {
      const payload = await readJsonBody(request);
      sentence = typeof payload.sentence === "string" ? payload.sentence.trim() : "";

      if (!sentence) {
        sendJson(response, 400, {
          success: false,
          error: "Please enter a sentence to parse."
        });
        return;
      }

      if (sentence.length > 400) {
        logSentenceRequest(request, {
          sentence,
          status: "rejected_too_long"
        });
        sendJson(response, 400, {
          success: false,
          error: "Please keep sentences under 400 characters for the demo site."
        });
        return;
      }

      const result = await runParser(sentence);
      logSentenceRequest(request, {
        sentence,
        normalizedSentence: typeof result.normalizedInput === "string" ? result.normalizedInput : "",
        interpretationCount: Number.isInteger(result.interpretationCount) ? result.interpretationCount : 0,
        status: result.success ? "parsed" : "no_parse",
        success: Boolean(result.success),
        unknownWords: Array.isArray(result.unknownWords) ? result.unknownWords : []
      });
      sendJson(response, 200, result);
    } catch (error) {
      logSentenceRequest(request, {
        sentence,
        status: "server_error",
        error: error.message || "Unexpected server error."
      });
      sendJson(response, 500, {
        success: false,
        error: error.message || "Unexpected server error."
      });
    }
    return;
  }

  if (request.method === "GET") {
    await serveStaticAsset(url.pathname, response);
    return;
  }

  sendText(response, 405, "Method not allowed");
});

server.listen(port, host, () => {
  console.log(`Sentence Reader web app running at http://${host}:${port}`);
});

function shutdown(signal) {
  console.log(`Received ${signal}, shutting down.`);
  server.close(() => {
    process.exit(0);
  });
}

process.on("SIGINT", () => shutdown("SIGINT"));
process.on("SIGTERM", () => shutdown("SIGTERM"));
