const form = document.querySelector("[data-parse-form]");
const sentenceInput = document.querySelector("#sentence-input");
const submitButton = document.querySelector("[data-submit-button]");
const resultsPanel = document.querySelector("[data-results-panel]");
const statusPill = document.querySelector("[data-status-pill]");
const interpretationCount = document.querySelector("[data-interpretation-count]");
const tokenCount = document.querySelector("[data-token-count]");
const unknownCount = document.querySelector("[data-unknown-count]");
const message = document.querySelector("[data-message]");
const tokenStrip = document.querySelector("[data-token-strip]");
const unknownStrip = document.querySelector("[data-unknown-strip]");
const interpretationGrid = document.querySelector("[data-interpretation-grid]");
const interpretationTemplate = document.querySelector("#interpretation-template");
const sampleButtons = document.querySelectorAll("[data-sample]");
let connectorLayoutFrame = 0;

const nodeHueMap = {
  S0: "355deg",
  S: "355deg",
  RelClause: "342deg",
  Inf: "330deg",
  NP: "36deg",
  Pron: "52deg",
  Name: "58deg",
  Det: "64deg",
  Num: "76deg",
  N: "86deg",
  VP: "140deg",
  V: "152deg",
  AdvP: "168deg",
  Adv: "168deg",
  AuxP: "176deg",
  Aux: "176deg",
  CopP: "196deg",
  Cop: "196deg",
  AdjP: "214deg",
  Adj: "214deg",
  PP: "240deg",
  P: "248deg",
  FOR: "262deg",
  TO: "282deg",
  Conj: "304deg",
  CoordConj: "304deg",
  ConjAdv: "318deg",
  SubordConj: "312deg",
  THEREFORE: "318deg",
  RelPron: "326deg",
  Neg: "332deg"
};

function setState(state, label) {
  resultsPanel.dataset.state = state;
  statusPill.textContent = label;
}

function resetResults() {
  interpretationCount.textContent = "0";
  tokenCount.textContent = "0";
  unknownCount.textContent = "0";
  message.textContent = "Choose a sentence or enter your own to see how the parser analyzes it.";
  tokenStrip.hidden = true;
  unknownStrip.hidden = true;
  tokenStrip.replaceChildren();
  unknownStrip.replaceChildren();
  interpretationGrid.replaceChildren();
}

function renderChips(container, values, className) {
  container.replaceChildren();

  for (const value of values) {
    const chip = document.createElement("span");
    chip.className = className;
    chip.textContent = value;
    container.append(chip);
  }

  container.hidden = values.length === 0;
}

function parseQuotedValue(source, cursor) {
  let value = "";
  let index = cursor + 1;

  while (index < source.length) {
    const character = source[index];
    if (character === "\\") {
      const next = source[index + 1];
      const replacements = {
        "\"": "\"",
        "\\": "\\",
        n: "\n",
        r: "\r",
        t: "\t"
      };

      value += replacements[next] || next || "";
      index += 2;
      continue;
    }

    if (character === "\"") {
      return {
        value,
        nextIndex: index + 1
      };
    }

    value += character;
    index += 1;
  }

  throw new Error("Unterminated quoted value in parse tree.");
}

function parseSExpression(source) {
  function skipWhitespace(index) {
    let cursor = index;
    while (cursor < source.length && /\s/.test(source[cursor])) {
      cursor += 1;
    }
    return cursor;
  }

  function readAtom(index) {
    let cursor = index;
    while (cursor < source.length && !/[\s()]/.test(source[cursor])) {
      cursor += 1;
    }
    return {
      value: source.slice(index, cursor),
      nextIndex: cursor
    };
  }

  function parseNode(index) {
    let cursor = skipWhitespace(index);
    if (source[cursor] !== "(") {
      throw new Error("Expected `(` in parse tree.");
    }

    cursor = skipWhitespace(cursor + 1);
    const labelToken = readAtom(cursor);
    const node = {
      label: labelToken.value,
      word: "",
      children: []
    };

    cursor = labelToken.nextIndex;

    while (cursor < source.length) {
      cursor = skipWhitespace(cursor);

      if (source[cursor] === ")") {
        return {
          node,
          nextIndex: cursor + 1
        };
      }

      if (source[cursor] === "(") {
        const child = parseNode(cursor);
        node.children.push(child.node);
        cursor = child.nextIndex;
        continue;
      }

      if (source[cursor] === "\"") {
        const word = parseQuotedValue(source, cursor);
        node.word = word.value;
        cursor = word.nextIndex;
        continue;
      }

      const atom = readAtom(cursor);
      if (atom.value) {
        node.children.push({
          label: atom.value,
          word: "",
          children: []
        });
      }
      cursor = atom.nextIndex;
    }

    throw new Error("Unterminated parse tree.");
  }

  const parsed = parseNode(0);
  return parsed.node;
}

function getNodeHue(label) {
  if (nodeHueMap[label]) {
    return nodeHueMap[label];
  }

  let hash = 0;
  for (const character of label) {
    hash = (hash * 33 + character.charCodeAt(0)) % 360;
  }
  return `${hash}deg`;
}

function layoutTreeConnectors(scope = interpretationGrid) {
  const groups = scope.querySelectorAll(".tree-children");

  for (const group of groups) {
    const edges = [...group.children].filter((child) => child.classList.contains("tree-edge"));
    if (edges.length === 0) {
      continue;
    }

    const first = edges[0];
    const last = edges[edges.length - 1];
    const start = first.offsetLeft + first.offsetWidth / 2;
    const end = group.clientWidth - (last.offsetLeft + last.offsetWidth / 2);

    group.style.setProperty("--branch-start", `${start}px`);
    group.style.setProperty("--branch-end", `${end}px`);
  }
}

function scheduleTreeConnectorLayout() {
  if (connectorLayoutFrame) {
    cancelAnimationFrame(connectorLayoutFrame);
  }

  connectorLayoutFrame = requestAnimationFrame(() => {
    connectorLayoutFrame = 0;
    layoutTreeConnectors();
  });
}

function renderTreeNode(node) {
  const element = document.createElement("div");
  element.className = "tree-node";
  element.style.setProperty("--node-hue", getNodeHue(node.label));

  const label = document.createElement("div");
  label.className = "tree-label";
  label.textContent = node.label;
  element.append(label);

  if (node.word) {
    const word = document.createElement("div");
    word.className = "tree-word";
    word.textContent = node.word;
    element.append(word);
    return element;
  }

  if (node.children.length > 0) {
    const children = document.createElement("div");
    children.className = `tree-children${node.children.length === 1 ? " is-single" : ""}`;

    for (const childNode of node.children) {
      const edge = document.createElement("div");
      edge.className = "tree-edge";
      edge.append(renderTreeNode(childNode));
      children.append(edge);
    }

    element.append(children);
  }

  return element;
}

function createInterpretationCard(interpretation, index) {
  const fragment = interpretationTemplate.content.cloneNode(true);
  const article = fragment.querySelector(".interpretation-card");
  const title = fragment.querySelector(".interpretation-card__title");
  const treeShell = fragment.querySelector(".tree-shell");
  const rawOutput = fragment.querySelector("pre");

  title.textContent = `Reading ${index + 1}`;
  rawOutput.textContent = interpretation;

  try {
    const tree = parseSExpression(interpretation);
    const root = document.createElement("div");
    root.className = "tree-root";
    root.append(renderTreeNode(tree));
    treeShell.append(root);
  } catch {
    const fallback = document.createElement("pre");
    fallback.className = "raw-fallback";
    fallback.textContent = interpretation;
    treeShell.append(fallback);
  }

  return article;
}

function renderSuccess(result) {
  interpretationCount.textContent = String(result.interpretationCount);
  tokenCount.textContent = String(result.tokens.length);
  unknownCount.textContent = String(result.unknownWords.length);

  renderChips(tokenStrip, result.tokens, "token");
  renderChips(unknownStrip, result.unknownWords, "unknown-token");

  if (result.interpretationCount > 1) {
    setState("success", "Ambiguous");
    message.textContent = `The parser found ${result.interpretationCount} valid readings for this sentence.`;
  } else {
    setState("success", "Parsed");
    message.textContent = "The parser found a single complete reading for this sentence.";
  }

  interpretationGrid.replaceChildren(
    ...result.interpretations.map((interpretation, index) =>
      createInterpretationCard(interpretation, index)
    )
  );

  scheduleTreeConnectorLayout();
}

function renderFailure(result) {
  interpretationCount.textContent = "0";
  tokenCount.textContent = String(result.tokens?.length || 0);
  unknownCount.textContent = String(result.unknownWords?.length || 0);

  renderChips(tokenStrip, result.tokens || [], "token");
  renderChips(unknownStrip, result.unknownWords || [], "unknown-token");
  interpretationGrid.replaceChildren();

  if (result.unknownWords?.length) {
    setState("warning", "Unknown words");
    message.textContent = `No complete parse was found. Unknown words: ${result.unknownWords.join(", ")}.`;
    return;
  }

  setState("warning", "No parse");
  message.textContent = result.error || "The grammar could not derive a full parse for this sentence.";
}

async function requestParse(sentence) {
  const response = await fetch("/api/parse", {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify({ sentence })
  });

  const payload = await response.json();
  if (!response.ok) {
    throw new Error(payload.error || "The parser request failed.");
  }

  return payload;
}

async function handleSubmit(event) {
  event?.preventDefault();

  const sentence = sentenceInput.value;
  if (!sentence.trim()) {
    setState("warning", "Need input");
    message.textContent = "Please enter a sentence before parsing.";
    return;
  }

  submitButton.disabled = true;
  setState("loading", "Parsing");
  message.textContent = "Running the C++ parser and assembling interpretations...";
  interpretationGrid.replaceChildren();

  try {
    const result = await requestParse(sentence);

    if (result.success) {
      renderSuccess(result);
    } else {
      renderFailure(result);
    }
  } catch (error) {
    interpretationCount.textContent = "0";
    tokenCount.textContent = "0";
    unknownCount.textContent = "0";
    tokenStrip.hidden = true;
    unknownStrip.hidden = true;
    interpretationGrid.replaceChildren();
    setState("error", "Server error");
    message.textContent = error.message;
  } finally {
    submitButton.disabled = false;
  }
}

form.addEventListener("submit", handleSubmit);

sentenceInput.addEventListener("keydown", (event) => {
  if ((event.metaKey || event.ctrlKey) && event.key === "Enter") {
    handleSubmit(event);
  }
});

for (const button of sampleButtons) {
  button.addEventListener("click", () => {
    sentenceInput.value = button.dataset.sample || "";
    sentenceInput.focus();
    handleSubmit();
  });
}

window.addEventListener("resize", scheduleTreeConnectorLayout);

resetResults();
handleSubmit();
