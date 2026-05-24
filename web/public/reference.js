const statusPill = document.querySelector("[data-reference-status]");
const ruleList = document.querySelector("[data-rule-list]");
const vocabularyGroups = document.querySelector("[data-vocabulary-groups]");

function setStatus(label, state = "") {
  statusPill.textContent = label;
  if (state) {
    statusPill.dataset.state = state;
  } else {
    delete statusPill.dataset.state;
  }
}

function createDirectoryNode(name) {
  return {
    name,
    type: "directory",
    children: new Map()
  };
}

function buildGrammarTree(rootLabel, files) {
  const root = createDirectoryNode(rootLabel || "languages");

  for (const file of files) {
    const segments = (file.relativePath || "").split("/").filter(Boolean);
    if (segments.length === 0) {
      continue;
    }

    let cursor = root;
    for (const segment of segments.slice(0, -1)) {
      if (!cursor.children.has(segment)) {
        cursor.children.set(segment, createDirectoryNode(segment));
      }

      cursor = cursor.children.get(segment);
    }

    cursor.children.set(segments.at(-1), {
      name: segments.at(-1),
      type: "file",
      rules: file.rules || []
    });
  }

  return root;
}

function sortTreeEntries(entries) {
  return [...entries].sort((left, right) => {
    if (left.type !== right.type) {
      return left.type === "directory" ? -1 : 1;
    }

    return left.name.localeCompare(right.name);
  });
}

function renderGrammarNode(node, depth = 0) {
  const details = document.createElement("details");
  details.className = `fs-node fs-node--${node.type}`;
  details.open = node.type === "directory" && depth === 0;

  const summary = document.createElement("summary");
  summary.className = "fs-node__summary";

  const icon = document.createElement("span");
  icon.className = "fs-node__icon";
  icon.textContent = node.type === "directory" ? "dir" : "txt";

  const name = document.createElement(node.type === "directory" ? "span" : "code");
  name.className = "fs-node__name";
  name.textContent = node.name;

  const meta = document.createElement("span");
  meta.className = "fs-node__meta";

  if (node.type === "directory") {
    const childCount = node.children.size;
    meta.textContent = `${childCount} item${childCount === 1 ? "" : "s"}`;
  } else {
    const ruleCount = node.rules.length;
    meta.textContent = `${ruleCount} rule${ruleCount === 1 ? "" : "s"}`;
  }

  summary.append(icon, name, meta);
  details.append(summary);

  if (node.type === "directory") {
    const children = document.createElement("div");
    children.className = "fs-node__children";

    for (const child of sortTreeEntries(node.children.values())) {
      children.append(renderGrammarNode(child, depth + 1));
    }

    details.append(children);
    return details;
  }

  const rules = document.createElement("div");
  rules.className = "fs-node__rules";

  for (const rule of node.rules) {
    const item = document.createElement("code");
    item.className = "fs-node__rule";
    item.textContent = rule;
    rules.append(item);
  }

  details.append(rules);
  return details;
}

function renderRules(files) {
  ruleList.replaceChildren();

  if (!Array.isArray(files) || files.length === 0) {
    const emptyState = document.createElement("p");
    emptyState.className = "reference-empty";
    emptyState.textContent = "No grammar files were found.";
    ruleList.append(emptyState);
    return;
  }

  const tree = buildGrammarTree("languages", files);
  for (const child of sortTreeEntries(tree.children.values())) {
    ruleList.append(renderGrammarNode(child, 0));
  }
}

function renderVocabulary(groups) {
  vocabularyGroups.replaceChildren();

  for (const group of groups) {
    const section = document.createElement("section");
    section.className = "reference-group";

    const header = document.createElement("div");
    header.className = "reference-group__header";

    const tag = document.createElement("h3");
    tag.className = "reference-group__title";
    tag.textContent = group.tag;

    const count = document.createElement("p");
    count.className = "reference-group__count";
    count.textContent = `${group.entries.length} entr${group.entries.length === 1 ? "y" : "ies"}`;

    header.append(tag, count);

    const entries = document.createElement("div");
    entries.className = "reference-group__entries";

    for (const entry of group.entries) {
      const chip = document.createElement("code");
      chip.className = "reference-chip";
      chip.textContent = entry;
      entries.append(chip);
    }

    section.append(header, entries);
    vocabularyGroups.append(section);
  }
}

async function loadReference() {
  setStatus("Loading", "loading");

  try {
    const response = await fetch("/api/reference");
    const payload = await response.json();

    if (!response.ok) {
      throw new Error(payload.error || "Could not load reference data.");
    }

    renderRules(payload.grammarFiles || []);
    renderVocabulary(payload.vocabularyGroups || []);
    setStatus("Loaded", "success");
  } catch (error) {
    ruleList.textContent = error.message;
    vocabularyGroups.replaceChildren();
    setStatus("Error", "error");
  }
}

loadReference();
