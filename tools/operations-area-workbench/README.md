# Operations Area Workbench

Tooling space for `operations_area` layout translation.

This workbench lives inside `soemdsp/tools` and remains intentionally outside
`soemdsp-sandbox`.

Open:

```text
public/index.html
```

Current baseline:

```text
The workbench has one primitive: an isolated node.
An isolated node is a draggable circle.
Double-click empty canvas to add one isolated node.
Left-click drag an isolated node to move it.
Click an isolated node to select it.
Shift-click an isolated node to add it to the current selection.
Dragging one selected isolated node moves all selected isolated nodes together.
Drag empty canvas to marquee-select isolated nodes.
Press Delete or Backspace to delete selected nodes.
Press Ctrl+A to select all isolated nodes.
Ctrl-drag an isolated node to snap its center to nearby box ghost points.
Isolated nodes do not snap to other isolated nodes.
The left tool rail shows inert box, circle, line, and triangle buttons.
With exactly two isolated nodes selected, the box button creates one box from those node positions.
Created boxes stay linked to the two source nodes and redraw when either node moves.
Created boxes show ghost points at four corners and center.
Ghost points are visual snap targets, not isolated nodes.
Only isolated nodes are selectable.
There is no box/grid primitive.
There is no spawned-object system.
There are no parent/child relationships.
There are no snapping links or persistent relation records.
Use Ctrl+Z to undo and Ctrl+Y to redo script edits when focus is not in a text field.
The script panel is hidden by default.
The script panel starts with an empty export.
Treat soemdsp-sandbox integration as a later copy/integration step.
```

Current script shape:

```js
const nodes = [
  { id: "node1", x: 96.0, y: 84.0 },
];

export { nodes };
```
