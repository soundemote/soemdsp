const defaultScript = "export { };";

const state = {
  history: [],
  historyIndex: -1,
  scriptVisible: false,
  splitDrag: null,
  splitRatio: 0.5,
};

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, Number.isFinite(value) ? value : min));
}

function scriptElement() {
  return document.getElementById("areaScript");
}

function snapshot() {
  return {
    script: scriptElement().value,
  };
}

function sameSnapshot(a, b) {
  return a?.script === b?.script;
}

function restoreSnapshot(nextSnapshot) {
  scriptElement().value = nextSnapshot?.script || defaultScript;
  render();
}

function recordHistory() {
  const nextSnapshot = snapshot();
  if (state.historyIndex >= 0 && sameSnapshot(state.history[state.historyIndex], nextSnapshot)) {
    renderHistoryButtons();
    return;
  }
  state.history = state.history.slice(0, state.historyIndex + 1);
  state.history.push(nextSnapshot);
  state.historyIndex = state.history.length - 1;
  renderHistoryButtons();
}

function render() {
  const script = scriptElement();
  if (!script.value.trim()) {
    script.value = defaultScript;
  }
  script.classList.remove("invalid");
  document.getElementById("areaReadout").value = "operations area ready";
  renderHistoryButtons();
}

function renderHistoryButtons() {
  document.getElementById("undoButton").disabled = state.historyIndex <= 0;
  document.getElementById("redoButton").disabled = state.historyIndex >= state.history.length - 1;
}

function renderScriptLayout() {
  const workbench = document.querySelector(".workbench");
  const button = document.getElementById("scriptToggleButton");
  workbench.classList.toggle("script-hidden", !state.scriptVisible);
  button.textContent = state.scriptVisible ? "Hide Script" : "Show Script";
  button.setAttribute("aria-pressed", String(state.scriptVisible));
  if (!state.scriptVisible) {
    workbench.style.removeProperty("--area-pane-width");
    workbench.style.removeProperty("--script-pane-width");
    return;
  }
  const ratio = clamp(state.splitRatio, 0.05, 0.95);
  state.splitRatio = ratio;
  workbench.style.setProperty("--area-pane-width", `${ratio}fr`);
  workbench.style.setProperty("--script-pane-width", `${1 - ratio}fr`);
}

function resetWorkbench() {
  scriptElement().value = defaultScript;
  render();
  recordHistory();
}

function undo() {
  if (state.historyIndex <= 0) {
    return;
  }
  state.historyIndex -= 1;
  restoreSnapshot(state.history[state.historyIndex]);
}

function redo() {
  if (state.historyIndex >= state.history.length - 1) {
    return;
  }
  state.historyIndex += 1;
  restoreSnapshot(state.history[state.historyIndex]);
}

function handleScriptInput() {
  render();
  recordHistory();
}

async function copyScript() {
  await navigator.clipboard.writeText(scriptElement().value);
}

function toggleScriptPane() {
  state.scriptVisible = !state.scriptVisible;
  renderScriptLayout();
}

function beginSplitDrag(event) {
  if (!state.scriptVisible || event.button !== 0) {
    return;
  }
  const workbench = document.querySelector(".workbench");
  const bounds = workbench.getBoundingClientRect();
  state.splitDrag = {
    left: bounds.left,
    pointerId: event.pointerId,
    width: bounds.width,
  };
  workbench.classList.add("resizing");
  event.currentTarget.setPointerCapture?.(event.pointerId);
  event.preventDefault();
}

function dragSplit(event) {
  const dragState = state.splitDrag;
  if (!dragState || dragState.pointerId !== event.pointerId) {
    return;
  }
  const ratio = (event.clientX - dragState.left) / Math.max(1, dragState.width);
  state.splitRatio = clamp(ratio, 0.05, 0.95);
  renderScriptLayout();
}

function endSplitDrag(event) {
  if (!state.splitDrag || state.splitDrag.pointerId !== event.pointerId) {
    return;
  }
  state.splitDrag = null;
  document.querySelector(".workbench").classList.remove("resizing");
}

function handleSplitterKeydown(event) {
  if (!state.scriptVisible) {
    return;
  }
  if (event.key === "ArrowLeft") {
    state.splitRatio = clamp(state.splitRatio - 0.025, 0.05, 0.95);
  } else if (event.key === "ArrowRight") {
    state.splitRatio = clamp(state.splitRatio + 0.025, 0.05, 0.95);
  } else {
    return;
  }
  event.preventDefault();
  renderScriptLayout();
}

function handleDocumentKeydown(event) {
  const target = event.target;
  const isEditing = target instanceof HTMLInputElement ||
    target instanceof HTMLTextAreaElement ||
    target instanceof HTMLSelectElement ||
    target?.isContentEditable;
  const modifierKey = event.ctrlKey || event.metaKey;
  const key = event.key.toLowerCase();
  if (!isEditing && modifierKey && !event.shiftKey && key === "z") {
    event.preventDefault();
    undo();
    return;
  }
  if (!isEditing && modifierKey && (key === "y" || (event.shiftKey && key === "z"))) {
    event.preventDefault();
    redo();
  }
}

document.addEventListener("pointermove", dragSplit);
document.addEventListener("pointerup", endSplitDrag);
document.addEventListener("pointercancel", endSplitDrag);
document.addEventListener("keydown", handleDocumentKeydown, { capture: true });
document.getElementById("scriptToggleButton").addEventListener("click", toggleScriptPane);
document.getElementById("splitter").addEventListener("pointerdown", beginSplitDrag);
document.getElementById("splitter").addEventListener("keydown", handleSplitterKeydown);
document.getElementById("undoButton").addEventListener("click", undo);
document.getElementById("redoButton").addEventListener("click", redo);
document.getElementById("resetButton").addEventListener("click", resetWorkbench);
document.getElementById("copyButton").addEventListener("click", copyScript);
scriptElement().addEventListener("input", handleScriptInput);
scriptElement().value = defaultScript;
renderScriptLayout();
render();
recordHistory();
