import { execFile } from "node:child_process";
import { existsSync } from "node:fs";
import { readdir, readFile } from "node:fs/promises";
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

function resolveExistingPath(candidates, type = "file") {
  for (const candidate of candidates) {
    if (!existsSync(candidate)) {
      continue;
    }

    if (type === "directory") {
      return candidate;
    }

    return candidate;
  }

  return "";
}

function resolveLanguageDataDirectory() {
  return resolveExistingPath([
    path.join(rootDir, "main", "language-data"),
    path.join(rootDir, "language-data"),
    path.join(rootDir, "build", "language-data")
  ], "directory");
}

function resolveGrammarSourceFile() {
  return resolveExistingPath([
    path.join(rootDir, "main", "src", "Grammar.cpp")
  ]);
}

function resolveVocabularyFile() {
  return resolveExistingPath([
    path.join(rootDir, "main", "language-data", "english", "vocab.txt"),
    path.join(rootDir, "language-data", "english", "vocab.txt"),
    path.join(rootDir, "build", "language-data", "english", "vocab.txt"),
    path.join(rootDir, "main", "data", "vocab.txt"),
    path.join(rootDir, "data", "vocab.txt"),
    path.join(rootDir, "build", "data", "vocab.txt")
  ]);
}

async function collectGrammarFiles(rootDirPath) {
  const entries = await readdir(rootDirPath, { withFileTypes: true });
  const files = [];

  for (const entry of entries) {
    const entryPath = path.join(rootDirPath, entry.name);
    if (entry.isDirectory()) {
      files.push(...await collectGrammarFiles(entryPath));
      continue;
    }

    if (entry.isFile() && entry.name.endsWith(".txt") && entry.name !== "MakeGrammar.txt") {
      files.push(entryPath);
    }
  }

  files.sort();
  return files;
}

async function loadGrammarFiles() {
  const languageDataDir = resolveLanguageDataDirectory();
  if (languageDataDir) {
    const entries = await readdir(languageDataDir, { withFileTypes: true });
    const grammarFiles = [];

    for (const entry of entries) {
      if (!entry.isDirectory()) {
        continue;
      }

      const grammarDir = path.join(languageDataDir, entry.name, "grammar");
      if (!existsSync(grammarDir)) {
        continue;
      }

      const files = await collectGrammarFiles(grammarDir);
      for (const file of files) {
        const source = await readFile(file, "utf8");
        const rules = [];
        for (const rawLine of source.split(/\r?\n/)) {
          const line = rawLine.replace(/#.*/, "").trim();
          if (line) {
            rules.push(line);
          }
        }

        grammarFiles.push({
          relativePath: [entry.name, path.relative(grammarDir, file).split(path.sep).join("/")].join("/"),
          rules
        });
      }
    }

    grammarFiles.sort((left, right) => left.relativePath.localeCompare(right.relativePath));

    if (grammarFiles.length > 0) {
      return {
        root: "",
        files: grammarFiles
      };
    }
  }

  const grammarSourcePath = resolveGrammarSourceFile();
  if (grammarSourcePath) {
    const source = await readFile(grammarSourcePath, "utf8");
    const stripped = stripCppComments(source);
    const match = stripped.match(/rules_string\s*=\s*\{([\s\S]*?)\};/);

    if (match) {
      return {
        root: "",
        files: [{
          relativePath: "english/Grammar.cpp",
          rules: [...match[1].matchAll(/"([^"]+)"/g)].map((entry) => entry[1].trim())
        }]
      };
    }
  }

  throw new Error("Could not locate grammar rule files.");
}

async function loadVocabularyGroups() {
  const vocabPath = resolveVocabularyFile();
  if (!vocabPath) {
    throw new Error("Could not locate vocab.txt");
  }

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

function parseVocabularyEntry(value) {
  const [lemmaPart, ...attributeParts] = value.split("|");
  const entry = {
    lemma: lemmaPart.trim()
  };

  for (const attributeGroup of attributeParts) {
    for (const token of attributeGroup.trim().split(/\s+/)) {
      const equalsIndex = token.indexOf("=");
      if (equalsIndex === -1) {
        continue;
      }

      const key = token.slice(0, equalsIndex).trim();
      const attributeValue = token.slice(equalsIndex + 1).trim();
      if (key && attributeValue) {
        entry[key] = attributeValue;
      }
    }
  }

  return entry;
}

async function loadVocabularyCatalog() {
  const vocabPath = resolveVocabularyFile();
  if (!vocabPath) {
    throw new Error("Could not locate vocab.txt");
  }

  const source = await readFile(vocabPath, "utf8");
  const surface = Object.create(null);
  const catalog = {
    surface,
    nouns: [],
    verbs: [],
    adjectives: []
  };

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

    if (!surface[tag]) {
      surface[tag] = [];
    }

    if (tag === "V") {
      catalog.verbs.push(parseVocabularyEntry(value));
      continue;
    }

    if (tag === "N") {
      catalog.nouns.push(parseVocabularyEntry(value));
      continue;
    }

    if (tag === "Adj") {
      catalog.adjectives.push(parseVocabularyEntry(value));
      continue;
    }

    for (const token of value.split(/\s+/)) {
      if (token) {
        surface[tag].push(token);
      }
    }
  }

  catalog.nameSet = new Set(surface.Name || []);
  return catalog;
}

function isVowel(character) {
  return ["a", "e", "i", "o", "u"].includes(character);
}

function endsWithAny(word, suffixes) {
  return suffixes.some((suffix) => word.endsWith(suffix));
}

function endsWithCvc(word) {
  if (word.length < 3 || word.length > 4) {
    return false;
  }

  if (endsWithAny(word, ["en", "er", "el"])) {
    return false;
  }

  const first = word[word.length - 3];
  const second = word[word.length - 2];
  const third = word[word.length - 1];

  return !isVowel(first)
    && isVowel(second)
    && !isVowel(third)
    && !["w", "x", "y"].includes(third);
}

function buildRegularPlural(lemma) {
  if (endsWithAny(lemma, ["s", "sh", "ch", "x", "z"])) {
    return `${lemma}es`;
  }

  if (lemma.length > 1 && lemma.endsWith("y") && !endsWithAny(lemma, ["ay", "ey", "iy", "oy", "uy"])) {
    return `${lemma.slice(0, -1)}ies`;
  }

  return `${lemma}s`;
}

function buildRegularPast(lemma) {
  if (lemma.length > 1 && lemma.endsWith("y") && !isVowel(lemma[lemma.length - 2])) {
    return `${lemma.slice(0, -1)}ied`;
  }

  if (lemma.endsWith("e")) {
    return `${lemma}d`;
  }

  if (endsWithCvc(lemma)) {
    return `${lemma}${lemma.at(-1)}ed`;
  }

  return `${lemma}ed`;
}

function buildRegularGerund(lemma) {
  if (lemma.endsWith("ie")) {
    return `${lemma.slice(0, -2)}ying`;
  }

  if (lemma.length > 1 && lemma.endsWith("e") && !lemma.endsWith("ee")) {
    return `${lemma.slice(0, -1)}ing`;
  }

  if (endsWithCvc(lemma)) {
    return `${lemma}${lemma.at(-1)}ing`;
  }

  return `${lemma}ing`;
}

function pickRandom(values) {
  if (!Array.isArray(values) || values.length === 0) {
    return "";
  }

  return values[Math.floor(Math.random() * values.length)];
}

function randomChance(probability) {
  return Math.random() < probability;
}

function availableWords(catalog, tag, preferred = []) {
  const values = catalog.surface[tag] || [];
  if (preferred.length === 0) {
    return values;
  }

  const preferredSet = new Set(preferred);
  const filtered = values.filter((value) => preferredSet.has(value));
  return filtered.length > 0 ? filtered : values;
}

function nounSurface(entry, plural) {
  if (!plural) {
    return entry.lemma;
  }

  return entry.plural || buildRegularPlural(entry.lemma);
}

function verbSurface(entry, form) {
  if (form === "past") {
    return entry.past || buildRegularPast(entry.lemma);
  }

  if (form === "part") {
    return entry.part || entry.past || buildRegularPast(entry.lemma);
  }

  if (form === "gerund") {
    return entry.gerund || buildRegularGerund(entry.lemma);
  }

  return entry.lemma;
}

function adjectiveSurface(entry, form) {
  if (form === "comparative") {
    return entry.comparative || entry.lemma;
  }

  if (form === "superlative") {
    return entry.superlative || entry.lemma;
  }

  return entry.lemma;
}

function capitalizeWord(word) {
  if (!word) {
    return word;
  }

  return `${word[0].toUpperCase()}${word.slice(1)}`;
}

function formatSentence(tokens, catalog) {
  const formatted = tokens.map((token, index) => {
    if (!token) {
      return token;
    }

    if (token === "i") {
      return "I";
    }

    if (catalog.nameSet.has(token)) {
      return capitalizeWord(token);
    }

    if (index === 0) {
      return capitalizeWord(token);
    }

    return token;
  });

  return `${formatted.join(" ")}.`;
}

function pickDeterminer(catalog, nextWord, plural) {
  const pluralChoices = availableWords(catalog, "Det", [
    "the", "these", "those", "some", "many", "several", "all"
  ]);
  const singularChoices = availableWords(catalog, "Det", [
    "the", "this", "that", "some", "each", "every"
  ]);

  if (plural) {
    return pickRandom(pluralChoices);
  }

  const wantsIndefiniteArticle = randomChance(0.28) && availableWords(catalog, "Det", ["a", "an"]).length > 0;
  if (wantsIndefiniteArticle) {
    return isVowel((nextWord || "").charAt(0)) ? "an" : "a";
  }

  return pickRandom(singularChoices);
}

function makeAdvP(catalog) {
  const adverbs = catalog.surface.Adv || [];
  return [pickRandom(adverbs)];
}

function makeAdjP(catalog) {
  const adjective = pickRandom(catalog.adjectives);
  if (!adjective) {
    return [];
  }

  if (randomChance(0.18)) {
    const form = randomChance(0.6) ? "comparative" : "superlative";
    return [adjectiveSurface(adjective, form)];
  }

  if (randomChance(0.24)) {
    const intensifiers = availableWords(catalog, "Adv", ["very", "really", "fairly", "rather", "especially", "truly"]);
    return [pickRandom(intensifiers), adjectiveSurface(adjective, "base")];
  }

  if (randomChance(0.28)) {
    const second = pickRandom(catalog.adjectives);
    if (second && second.lemma !== adjective.lemma) {
      return [adjectiveSurface(adjective, "base"), adjectiveSurface(second, "base")];
    }
  }

  return [adjectiveSurface(adjective, "base")];
}

function makeBaseNp(catalog) {
  const subjectPronouns = availableWords(catalog, "Pron", [
    "i", "you", "we", "they", "he", "she", "it", "someone", "somebody", "everyone", "everybody", "nobody"
  ]);

  if (randomChance(0.22) && subjectPronouns.length > 0) {
    return [pickRandom(subjectPronouns)];
  }

  if (randomChance(0.18) && (catalog.surface.Name || []).length > 0) {
    return [pickRandom(catalog.surface.Name)];
  }

  const nounEntry = pickRandom(catalog.nouns);
  if (!nounEntry) {
    return ["someone"];
  }

  const useNumber = randomChance(0.25) && (catalog.surface.Num || []).length > 0;
  const number = useNumber
    ? pickRandom(availableWords(catalog, "Num", ["one", "two", "three", "four", "five", "six"]))
    : "";
  const plural = useNumber ? number !== "one" : randomChance(0.22);
  const adjectivePhrase = randomChance(0.45) ? makeAdjP(catalog) : [];
  const headWord = adjectivePhrase[0] || number || nounEntry.lemma;
  const determiner = useNumber
    ? (randomChance(0.35) ? pickDeterminer(catalog, headWord, plural) : "")
    : (randomChance(0.8) ? pickDeterminer(catalog, headWord, plural) : "");
  const nounWord = nounSurface(nounEntry, plural);

  return [
    ...(determiner ? [determiner] : []),
    ...(number ? [number] : []),
    ...adjectivePhrase,
    nounWord
  ];
}

function makeInfVp(catalog) {
  const verb = pickRandom(catalog.verbs);
  if (!verb) {
    return ["wait"];
  }

  const tokens = [verbSurface(verb, "base")];
  if (randomChance(0.55)) {
    tokens.push(...makeBaseNp(catalog));
  }
  return tokens;
}

function makePp(catalog, depth) {
  if (depth < 2 && randomChance(0.12) && (catalog.surface.FOR || []).length > 0 && (catalog.surface.TO || []).length > 0) {
    return [
      pickRandom(catalog.surface.FOR),
      ...makeBaseNp(catalog),
      pickRandom(catalog.surface.TO),
      ...makeInfVp(catalog)
    ];
  }

  const preposition = pickRandom(availableWords(catalog, "P", [
    "in", "on", "at", "with", "near", "behind", "before", "after", "under",
    "around", "inside", "outside", "beside", "between", "among"
  ]));
  return [preposition, ...makeNp(catalog, depth + 1, { allowRelative: false, allowCoordination: false })];
}

function makeRelClause(catalog, depth) {
  const relPronoun = pickRandom(catalog.surface.RelPron || []);
  return [relPronoun, ...makeRelativeVp(catalog, depth + 1)];
}

function makeRelativeVp(catalog, depth) {
  const verb = pickRandom(catalog.verbs);
  if (!verb) {
    return ["waited"];
  }

  if (randomChance(0.7)) {
    return [verbSurface(verb, "past"), ...makeBaseNp(catalog)];
  }

  if (randomChance(0.15)) {
    return [pickRandom(availableWords(catalog, "Cop", ["was", "seemed", "appeared"])) || "was", ...makeAdjP(catalog)];
  }

  return [verbSurface(verb, "past")];
}

function makeNp(catalog, depth = 0, options = {}) {
  const {
    allowRelative = true,
    allowCoordination = true
  } = options;

  let tokens = makeBaseNp(catalog);

  if (depth < 2 && allowRelative && randomChance(0.08)) {
    tokens = [...tokens, ...makeRelClause(catalog, depth + 1)];
  }

  if (depth < 2 && randomChance(0.08)) {
    tokens = [...tokens, ...makePp(catalog, depth + 1)];
  }

  if (depth < 1 && allowCoordination && randomChance(0.04)) {
    const conjunction = pickRandom(availableWords(catalog, "AND", ["and"])) || "and";
    tokens = [
      ...tokens,
      conjunction,
      ...makeNp(catalog, depth + 1, { allowRelative: false, allowCoordination: false })
    ];
  }

  return tokens;
}

function makeVp(catalog, depth = 0, options = {}) {
  const {
    allowCoordination = true
  } = options;

  const templatePool = [
    "intransitive",
    "transitive",
    "copAdj",
    "copPp",
    "modal",
    "toInfinitive"
  ];

  if (depth < 1 && allowCoordination) {
    templatePool.push("coordinated");
  }

  const template = pickRandom(templatePool);

  if (template === "coordinated") {
    const conjunction = pickRandom([
      ...availableWords(catalog, "AND", ["and"]),
      ...availableWords(catalog, "BUT", ["but"]),
      ...availableWords(catalog, "OR", ["or"])
    ]);

    return [
      ...makeVp(catalog, depth + 1, { allowCoordination: false }),
      conjunction || "and",
      ...makeVp(catalog, depth + 1, { allowCoordination: false })
    ];
  }

  if (template === "copAdj") {
    const cop = pickRandom(availableWords(catalog, "Cop", [
      "is", "was", "were", "seemed", "became", "remained", "appeared", "looked", "sounded", "stayed"
    ]));
    return [cop || "was", ...makeAdjP(catalog)];
  }

  if (template === "copPp") {
    const cop = pickRandom(availableWords(catalog, "Cop", [
      "is", "was", "were", "remained", "stayed"
    ]));
    const locativePrep = pickRandom(availableWords(catalog, "P", [
      "in", "on", "at", "near", "behind", "under", "inside", "outside", "beside"
    ]));
    return [cop || "was", locativePrep || "in", ...makeNp(catalog, depth + 1, { allowRelative: false, allowCoordination: false })];
  }

  if (template === "modal") {
    const modal = pickRandom(availableWords(catalog, "Aux", [
      "can", "could", "may", "might", "must", "shall", "should", "will", "would"
    ]));
    const verb = pickRandom(catalog.verbs);
    const tokens = [modal || "could"];

    if (randomChance(0.38)) {
      tokens.push(...makeAdvP(catalog));
    }

    tokens.push(verbSurface(verb, "base"));

    if (randomChance(0.62)) {
      tokens.push(...makeBaseNp(catalog));
    }

    return tokens;
  }

  if (template === "toInfinitive") {
    const controlVerb = pickRandom(
      catalog.verbs.filter((entry) =>
        ["agree", "begin", "continue", "decide", "hope", "learn", "plan", "prefer", "start", "try"].includes(entry.lemma)
      )
    ) || pickRandom(catalog.verbs);

    return [
      verbSurface(controlVerb, "past"),
      pickRandom(catalog.surface.TO || []) || "to",
      ...makeInfVp(catalog)
    ];
  }

  const verb = pickRandom(catalog.verbs);
  const tokens = [verbSurface(verb, "past")];

  if (template === "transitive") {
    tokens.push(...makeBaseNp(catalog));
  }

  if (depth < 2 && randomChance(0.2)) {
    tokens.push(...makePp(catalog, depth + 1));
  }

  if (depth < 2 && randomChance(0.15)) {
    tokens.push(...makeAdvP(catalog));
  }

  return tokens;
}

function makeClause(catalog, depth = 0) {
  return [...makeNp(catalog, depth), ...makeVp(catalog, depth)];
}

function buildRandomSentenceCandidate(catalog, depth = 0) {
  const templates = ["clause", "frontedPp", "frontedAdv"];

  const template = pickRandom(templates);

  if (template === "frontedPp") {
    return [...makePp(catalog, depth + 1), ...makeClause(catalog, depth + 1)];
  }

  if (template === "frontedAdv") {
    return [...makeAdvP(catalog), ...makeClause(catalog, depth + 1)];
  }

  return makeClause(catalog, depth + 1);
}

async function generateRandomSentenceResult() {
  const catalog = await loadVocabularyCatalog();

  for (let attempt = 0; attempt < 40; attempt += 1) {
    const candidate = formatSentence(buildRandomSentenceCandidate(catalog), catalog);
    const result = await runParser(candidate);

    if (result.success) {
      return result;
    }
  }

  throw new Error("Could not generate a parseable random sentence.");
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
      const [grammarReference, vocabularyGroups] = await Promise.all([
        loadGrammarFiles(),
        loadVocabularyGroups()
      ]);

      sendJson(response, 200, {
        grammarRoot: grammarReference.root,
        grammarFiles: grammarReference.files,
        grammarRules: grammarReference.files.flatMap((file) => file.rules),
        vocabularyGroups
      });
    } catch (error) {
      sendJson(response, 500, {
        error: error.message || "Could not load reference data."
      });
    }
    return;
  }

  if (request.method === "GET" && url.pathname === "/api/random-sentence") {
    try {
      const result = await generateRandomSentenceResult();
      sendJson(response, 200, result);
    } catch (error) {
      sendJson(response, 500, {
        success: false,
        error: error.message || "Could not generate a random sentence."
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
