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

function renderRules(rules) {
  ruleList.replaceChildren();

  for (const rule of rules) {
    const item = document.createElement("article");
    item.className = "reference-item";

    const code = document.createElement("code");
    code.className = "reference-item__code";
    code.textContent = rule;

    item.append(code);
    ruleList.append(item);
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

    renderRules(payload.grammarRules || []);
    renderVocabulary(payload.vocabularyGroups || []);
    setStatus("Loaded", "success");
  } catch (error) {
    ruleList.textContent = error.message;
    vocabularyGroups.replaceChildren();
    setStatus("Error", "error");
  }
}

loadReference();
