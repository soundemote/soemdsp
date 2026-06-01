# operations_area.hpp to JavaScript Mapping

Source header:

```text
include/soemdsp/utility/operations_area.hpp
```

JavaScript model:

```text
tools/operations-area-workbench/public/operations-area.js
```

Mapping:

```text
soemdsp::area::Anchor          -> Anchor
Point<value_t>                 -> Point
Line<value_t>                  -> Line
Bound<value_t>                 -> Bound
one layout grid primitive      -> GridBox
Grid                           -> Grid
Anchor::TL                     -> Anchor.TL
Anchor::TR                     -> Anchor.TR
Anchor::C                      -> Anchor.C
Anchor::BL                     -> Anchor.BL
Anchor::BR                     -> Anchor.BR
bound.point(anchor)            -> bound.point(anchor)
GridBox(name, bound, cols, rows)-> new GridBox(name, bound, cols, rows)
GridBox::from rect             -> GridBox.fromRect(name, { x, y, w, h, columns, rows })
gridBox.point(anchor)          -> gridBox.point(anchor)
gridBox.toRect()               -> gridBox.toRect()
grid.cell(row, column)         -> grid.cell(row, column)
grid.point(row, column, anchor)-> grid.point(row, column, anchor)
```

Notes:

```text
The JavaScript model is browser tooling, not production DSP runtime code.
The workbench currently has one primitive: an isolated node.
An isolated node is a draggable circle.
Double-clicking empty canvas adds one isolated node.
Left-click dragging an isolated node moves it.
Clicking an isolated node selects it.
Shift-clicking an isolated node adds it to the current selection.
Dragging one selected isolated node moves all selected isolated nodes together.
Dragging empty canvas marquee-selects isolated nodes.
Delete or Backspace removes selected isolated nodes.
Ctrl+A selects all isolated nodes.
Ctrl-dragging an isolated node snaps its center to nearby box ghost points.
Isolated nodes do not snap to other isolated nodes.
With exactly two isolated nodes selected, the box tool creates one box from those node positions.
Created boxes stay linked to the two source nodes and redraw when either node moves.
Created boxes show ghost points at four corners and center.
Ghost points are visual snap targets, not isolated nodes.
Only isolated nodes are selectable.
The current workbench baseline has no box/grid primitive.
The current workbench baseline has no spawned-object system.
The current workbench baseline has no parent/child relationships.
The current workbench baseline has no snapping links or persistent relation records.
Ctrl+Z and Ctrl+Y call workbench undo/redo when focus is not inside editable text.
The script panel is hidden by default.
The model keeps method names close to the C++ header where JavaScript can do so clearly.
```
