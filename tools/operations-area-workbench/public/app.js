const uiItemWidth = 120;
const uiItemHeight = 44;

const state = {
  history: [],
  historyIndex: -1,
  contextPoint: null,
  nextUiItemNumber: 1,
  scriptVisible: false,
  splitDrag: null,
  splitRatio: 0.5,
  uiItems: [],
};

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, Number.isFinite(value) ? value : min));
}

function cloneUiItems(uiItems) {
  return uiItems.map((item) => ({ ...item }));
}

function scriptElement() {
  return document.getElementById("areaScript");
}

function areaCanvasElement() {
  return document.getElementById("areaCanvas");
}

function actionsMenuElement() {
  return document.getElementById("actionsMenu");
}

function scriptFromState() {
  const uiItemsJson = JSON.stringify(state.uiItems, null, 2);
  return `const uiItems = ${uiItemsJson};\n\nexport { uiItems };\n`;
}

function syncScriptFromState() {
  scriptElement().value = scriptFromState();
}

function snapshot() {
  return {
    script: scriptElement().value,
    uiItems: cloneUiItems(state.uiItems),
    nextUiItemNumber: state.nextUiItemNumber,
  };
}

function sameSnapshot(a, b) {
  return a?.script === b?.script &&
    JSON.stringify(a?.uiItems) === JSON.stringify(b?.uiItems);
}

function restoreSnapshot(nextSnapshot) {
  state.uiItems = cloneUiItems(nextSnapshot?.uiItems || []);
  state.nextUiItemNumber = nextSnapshot?.nextUiItemNumber || 1;
  scriptElement().value = nextSnapshot?.script || scriptFromState();
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
    syncScriptFromState();
  }
  renderUiItems();
  script.classList.remove("invalid");
  document.getElementById("areaReadout").value = "operations area ready";
  renderHistoryButtons();
}

function renderUiItems() {
  const areaCanvas = areaCanvasElement();
  areaCanvas.querySelectorAll(".ui-item").forEach((element) => element.remove());
  for (const item of state.uiItems) {
    const element = document.createElement("div");
    element.className = "ui-item";
    element.style.left = `${item.x}px`;
    element.style.top = `${item.y}px`;
    element.textContent = item.label;
    element.setAttribute("aria-label", item.label);
    areaCanvas.append(element);
  }
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
  state.uiItems = [];
  state.nextUiItemNumber = 1;
  syncScriptFromState();
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

function canvasPointFromEvent(event) {
  const bounds = areaCanvasElement().getBoundingClientRect();
  return {
    x: clamp(event.clientX - bounds.left, 0, bounds.width),
    y: clamp(event.clientY - bounds.top, 0, bounds.height),
  };
}

function showActionsMenu(event) {
  event.preventDefault();
  const point = canvasPointFromEvent(event);
  state.contextPoint = point;

  const menu = actionsMenuElement();
  menu.style.left = `${point.x}px`;
  menu.style.top = `${point.y}px`;
  menu.hidden = false;
}

function hideActionsMenu() {
  actionsMenuElement().hidden = true;
}

function addContextItemToUi() {
  const point = state.contextPoint || { x: 0, y: 0 };
  const bounds = areaCanvasElement().getBoundingClientRect();
  const id = `uiItem${state.nextUiItemNumber}`;
  state.nextUiItemNumber += 1;
  state.uiItems.push({
    id,
    label: `UI ${state.uiItems.length + 1}`,
    x: Math.round(clamp(point.x, 0, Math.max(0, bounds.width - uiItemWidth))),
    y: Math.round(clamp(point.y, 0, Math.max(0, bounds.height - uiItemHeight))),
    w: uiItemWidth,
    h: uiItemHeight,
  });
  syncScriptFromState();
  hideActionsMenu();
  render();
  recordHistory();
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
document.addEventListener("pointerdown", (event) => {
  if (!actionsMenuElement().hidden && !actionsMenuElement().contains(event.target)) {
    hideActionsMenu();
  }
});
document.getElementById("scriptToggleButton").addEventListener("click", toggleScriptPane);
document.getElementById("splitter").addEventListener("pointerdown", beginSplitDrag);
document.getElementById("splitter").addEventListener("keydown", handleSplitterKeydown);
document.getElementById("undoButton").addEventListener("click", undo);
document.getElementById("redoButton").addEventListener("click", redo);
document.getElementById("resetButton").addEventListener("click", resetWorkbench);
document.getElementById("copyButton").addEventListener("click", copyScript);
areaCanvasElement().addEventListener("contextmenu", showActionsMenu);
document.getElementById("addToUiButton").addEventListener("click", addContextItemToUi);
scriptElement().addEventListener("input", handleScriptInput);
syncScriptFromState();
renderScriptLayout();
render();
recordHistory();
