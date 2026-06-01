export const Anchor = Object.freeze({
  TL: "TL",
  T: "T",
  TR: "TR",
  L: "L",
  C: "C",
  R: "R",
  BL: "BL",
  B: "B",
  BR: "BR",
});

const horizontalResizeAnchors = new Set([Anchor.TR, Anchor.R, Anchor.BR]);
const horizontalCenterAnchors = new Set([Anchor.T, Anchor.B, Anchor.C]);
const verticalResizeAnchors = new Set([Anchor.BR, Anchor.B, Anchor.BL]);
const verticalCenterAnchors = new Set([Anchor.R, Anchor.L, Anchor.C]);

function finiteNumber(value, fallback = 0) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function normalizeAnchor(anchor) {
  if (Object.values(Anchor).includes(anchor)) {
    return anchor;
  }
  throw new Error(`Unknown Anchor: ${anchor}`);
}

export class Point {
  constructor(x = 0, y = 0) {
    this.x = finiteNumber(x);
    this.y = finiteNumber(y);
  }

  static center(p1, p2) {
    return new Point((p1.x + p2.x) * 0.5, (p1.y + p2.y) * 0.5);
  }

  static interpolate(p1, p2, ratio) {
    return new Point(
      p1.x + (p2.x - p1.x) * ratio,
      p1.y + (p2.y - p1.y) * ratio,
    );
  }

  static from(value) {
    if (value instanceof Point) {
      return value.clone();
    }
    return new Point(value?.x, value?.y);
  }

  move(x, y) {
    this.x += finiteNumber(x);
    this.y += finiteNumber(y);
    return this;
  }

  set(x, y) {
    return this.move(finiteNumber(x) - this.x, finiteNumber(y) - this.y);
  }

  clone() {
    return new Point(this.x, this.y);
  }

  toJSON() {
    return { x: this.x, y: this.y };
  }
}

export class Line {
  constructor(start = new Point(), end = new Point()) {
    this.start = Point.from(start);
    this.end = Point.from(end);
  }

  getDistance(point) {
    const p = Point.from(point);
    const dx = this.end.x - this.start.x;
    const dy = this.end.y - this.start.y;
    const numerator = Math.abs(
      dy * p.x - dx * p.y + this.end.x * this.start.y - this.end.y * this.start.x,
    );
    const denominator = Math.sqrt(dx * dx + dy * dy);
    return denominator < Number.EPSILON ? 0 : numerator / denominator;
  }
}

export class Bound {
  constructor(topLeft = new Point(), baseRight = new Point()) {
    this.x = 0;
    this.y = 0;
    this.w = 0;
    this.h = 0;
    this.setCorners(topLeft, baseRight);
  }

  static fromCorners(topLeft = new Point(), baseRight = new Point()) {
    return new Bound(topLeft, baseRight);
  }

  static fromReference(reference = new Point(), width = 0, height = 0, anchor = Anchor.TL) {
    return new Bound().setReference(reference, width, height, anchor);
  }

  static fromRect(rect = {}) {
    const x = finiteNumber(rect.x);
    const y = finiteNumber(rect.y);
    return Bound.fromReference(new Point(x, y), rect.w, rect.h, Anchor.TL);
  }

  set(topLeft, baseRight) {
    return this.setCorners(topLeft, baseRight);
  }

  setCorners(topLeft, baseRight) {
    const tl = Point.from(topLeft);
    const br = Point.from(baseRight);
    this.x = tl.x;
    this.y = tl.y;
    this.w = Math.abs(br.x - tl.x);
    this.h = Math.abs(br.y - tl.y);
    return this;
  }

  setReference(reference, width = 0, height = 0, anchor = Anchor.TL) {
    const ref = Point.from(reference);
    const normalizedAnchor = normalizeAnchor(anchor);
    this.w = Math.abs(finiteNumber(width));
    this.h = Math.abs(finiteNumber(height));

    switch (normalizedAnchor) {
      case Anchor.TL:
        this.x = ref.x;
        this.y = ref.y;
        break;
      case Anchor.T:
        this.x = ref.x - this.w * 0.5;
        this.y = ref.y;
        break;
      case Anchor.TR:
        this.x = ref.x - this.w;
        this.y = ref.y;
        break;
      case Anchor.R:
        this.x = ref.x - this.w;
        this.y = ref.y - this.h * 0.5;
        break;
      case Anchor.BL:
        this.x = ref.x;
        this.y = ref.y - this.h;
        break;
      case Anchor.B:
        this.x = ref.x - this.w * 0.5;
        this.y = ref.y - this.h;
        break;
      case Anchor.BR:
        this.x = ref.x - this.w;
        this.y = ref.y - this.h;
        break;
      case Anchor.L:
        this.x = ref.x;
        this.y = ref.y - this.h * 0.5;
        break;
      case Anchor.C:
        this.x = ref.x - this.w * 0.5;
        this.y = ref.y - this.h * 0.5;
        break;
    }

    return this;
  }

  setWidth(width, anchor = Anchor.TL) {
    this.adjustWidth(Math.abs(finiteNumber(width)) - this.w, anchor);
    return this;
  }

  setHeight(height, anchor = Anchor.TL) {
    this.adjustHeight(Math.abs(finiteNumber(height)) - this.h, anchor);
    return this;
  }

  point(anchor = Anchor.TL) {
    switch (normalizeAnchor(anchor)) {
      case Anchor.TL:
        return new Point(this.x, this.y);
      case Anchor.T:
        return new Point(this.x + this.w * 0.5, this.y);
      case Anchor.TR:
        return new Point(this.x + this.w, this.y);
      case Anchor.L:
        return new Point(this.x, this.y + this.h * 0.5);
      case Anchor.C:
        return new Point(this.x + this.w * 0.5, this.y + this.h * 0.5);
      case Anchor.R:
        return new Point(this.x + this.w, this.y + this.h * 0.5);
      case Anchor.BL:
        return new Point(this.x, this.y + this.h);
      case Anchor.B:
        return new Point(this.x + this.w * 0.5, this.y + this.h);
      case Anchor.BR:
        return new Point(this.x + this.w, this.y + this.h);
    }
  }

  splitPx(pixels, anchor = Anchor.T) {
    switch (normalizeAnchor(anchor)) {
      case Anchor.T:
      case Anchor.B:
        return this.splitRatio(finiteNumber(pixels) / this.h, anchor);
      case Anchor.L:
      case Anchor.R:
        return this.splitRatio(finiteNumber(pixels) / this.w, anchor);
      default:
        throw new Error("splitPx supports Anchor.T, Anchor.B, Anchor.L, or Anchor.R");
    }
  }

  splitRatio(ratio, anchor) {
    const splitBound = new Bound();
    switch (normalizeAnchor(anchor)) {
      case Anchor.T:
        this.splitHorizontal(ratio, splitBound, this);
        break;
      case Anchor.B:
        this.splitHorizontal(ratio, this, splitBound);
        break;
      case Anchor.L:
        this.splitVertical(ratio, splitBound, this);
        break;
      case Anchor.R:
        this.splitVertical(ratio, this, splitBound);
        break;
      default:
        throw new Error("splitRatio supports Anchor.T, Anchor.B, Anchor.L, or Anchor.R");
    }
    return splitBound;
  }

  move(xAdjust, yAdjust) {
    this.x += finiteNumber(xAdjust);
    this.y += finiteNumber(yAdjust);
    return this;
  }

  resize(width, height, anchor = Anchor.TL) {
    if (width !== 0) {
      this.adjustWidth(finiteNumber(width), anchor);
    }
    if (height !== 0) {
      this.adjustHeight(finiteNumber(height), anchor);
    }
    return this;
  }

  size(width, height, anchor = Anchor.TL) {
    return this.resize(finiteNumber(width) - this.w, finiteNumber(height) - this.h, anchor);
  }

  square(anchor = Anchor.TL) {
    if (this.w > this.h) {
      this.adjustWidth(this.h - this.w, anchor);
    } else {
      this.adjustHeight(this.w - this.h, anchor);
    }
    return this;
  }

  clone() {
    return Bound.fromReference(this.point(Anchor.TL), this.w, this.h, Anchor.TL);
  }

  toRect() {
    return { x: this.x, y: this.y, w: this.w, h: this.h };
  }

  toJSON() {
    return this.toRect();
  }

  adjustWidth(adjustment, anchor = Anchor.TL) {
    const normalizedAnchor = normalizeAnchor(anchor);
    const delta = finiteNumber(adjustment);
    if (horizontalResizeAnchors.has(normalizedAnchor)) {
      this.x -= delta;
    } else if (horizontalCenterAnchors.has(normalizedAnchor)) {
      this.x -= delta * 0.5;
    }
    this.w = Math.abs(this.w + delta);
  }

  adjustHeight(adjustment, anchor = Anchor.TL) {
    const normalizedAnchor = normalizeAnchor(anchor);
    const delta = finiteNumber(adjustment);
    if (verticalResizeAnchors.has(normalizedAnchor)) {
      this.y -= delta;
    } else if (verticalCenterAnchors.has(normalizedAnchor)) {
      this.y -= delta * 0.5;
    }
    this.h = Math.abs(this.h + delta);
  }

  splitHorizontal(ratio, boundTop, boundBase) {
    const tl = this.point(Anchor.TL);
    const tr = this.point(Anchor.TR);
    const bl = this.point(Anchor.BL);
    const br = this.point(Anchor.BR);
    if (ratio <= 0) {
      boundTop.setCorners(tl, tr);
      boundBase.setCorners(tl, br);
      return;
    }
    if (ratio >= 1) {
      boundTop.setCorners(tl, br);
      boundBase.setCorners(bl, br);
      return;
    }

    const p = Point.interpolate(tl, bl, ratio);
    boundBase.setCorners(p, br);
    boundTop.setCorners(tl, boundBase.point(Anchor.TR));
  }

  splitVertical(ratio, boundLeft, boundRight) {
    const tl = this.point(Anchor.TL);
    const tr = this.point(Anchor.TR);
    const bl = this.point(Anchor.BL);
    const br = this.point(Anchor.BR);
    if (ratio <= 0) {
      boundLeft.setCorners(tl, bl);
      boundRight.setCorners(tl, br);
      return;
    }
    if (ratio >= 1) {
      boundLeft.setCorners(tl, br);
      boundRight.setCorners(tr, br);
      return;
    }

    const p = Point.interpolate(bl, br, ratio);
    boundLeft.setCorners(tl, p);
    boundRight.setCorners(boundLeft.point(Anchor.TR), br);
  }
}

export class AreaBox {
  constructor(name = "areaBox", bound = new Bound()) {
    this.name = String(name || "areaBox");
    this.bound = bound instanceof Bound ? bound.clone() : Bound.fromRect(bound);
  }

  static fromCorners(name, topLeft = new Point(), baseRight = new Point()) {
    return new AreaBox(name, Bound.fromCorners(topLeft, baseRight));
  }

  static fromReference(name, reference = new Point(), width = 0, height = 0, anchor = Anchor.TL) {
    return new AreaBox(name, Bound.fromReference(reference, width, height, anchor));
  }

  static fromRect(name, rect = {}) {
    return new AreaBox(name, Bound.fromRect(rect));
  }

  setBound(bound) {
    this.bound = bound instanceof Bound ? bound.clone() : Bound.fromRect(bound);
    return this;
  }

  point(anchor = Anchor.TL) {
    return this.bound.point(anchor);
  }

  move(xAdjust, yAdjust) {
    this.bound.move(xAdjust, yAdjust);
    return this;
  }

  resize(width, height, anchor = Anchor.TL) {
    this.bound.resize(width, height, anchor);
    return this;
  }

  size(width, height, anchor = Anchor.TL) {
    this.bound.size(width, height, anchor);
    return this;
  }

  square(anchor = Anchor.TL) {
    this.bound.square(anchor);
    return this;
  }

  toRect() {
    return this.bound.toRect();
  }

  clone() {
    return new AreaBox(this.name, this.bound);
  }

  toJSON() {
    return { name: this.name, bound: this.bound.toRect() };
  }
}

export class GridBox extends AreaBox {
  constructor(name = "grid", bound = new Bound(), columns = 12, rows = 8) {
    super(name, bound);
    this.columns = Math.max(1, Math.trunc(finiteNumber(columns, 12)));
    this.rows = Math.max(1, Math.trunc(finiteNumber(rows, 8)));
  }

  static fromRect(name, rect = {}) {
    const gridBox = new GridBox(
      name,
      Bound.fromRect(rect),
      rect.columns,
      rect.rows,
    );
    return gridBox;
  }

  toJSON() {
    return {
      name: this.name,
      bound: this.bound.toRect(),
      columns: this.columns,
      rows: this.rows,
    };
  }
}

export class Grid {
  constructor(rows, columns, boundOrTopLeft, baseRight) {
    this.grid = [];
    this.b = new Bound();
    this.initCellWidth = 0;
    this.initCellHeight = 0;

    if (boundOrTopLeft instanceof Bound && baseRight === undefined) {
      this.reset(rows, columns, boundOrTopLeft.point(Anchor.TL), boundOrTopLeft.point(Anchor.BR));
    } else {
      this.reset(rows, columns, boundOrTopLeft, baseRight);
    }
  }

  reset(rows, columns, topLeft, baseRight) {
    const rowCount = Math.max(0, Math.trunc(finiteNumber(rows)));
    const columnCount = Math.max(0, Math.trunc(finiteNumber(columns)));
    this.b.setCorners(topLeft, baseRight);
    this.initCellWidth = columnCount === 0 ? 0 : this.b.w / columnCount;
    this.initCellHeight = rowCount === 0 ? 0 : this.b.h / rowCount;
    this.grid = Array.from({ length: rowCount }, (_, row) => {
      return Array.from({ length: columnCount }, (_, column) => {
        return Bound.fromReference(
          new Point(
            this.b.x + this.initCellWidth * column,
            this.b.y + this.initCellHeight * row,
          ),
          this.initCellWidth,
          this.initCellHeight,
          Anchor.TL,
        );
      });
    });
  }

  sizeCells(width, height, anchor = Anchor.TL) {
    for (const row of this.grid) {
      for (const cell of row) {
        cell.size(width, height, anchor);
      }
    }
  }

  squareCells(anchor = Anchor.TL) {
    for (const row of this.grid) {
      for (const cell of row) {
        cell.square(anchor);
      }
    }
  }

  calculateCellBound(row, column) {
    return Bound.fromReference(
      new Point(
        this.b.x + this.initCellWidth * column,
        this.b.y + this.initCellHeight * row,
      ),
      this.initCellWidth,
      this.initCellHeight,
      Anchor.TL,
    );
  }

  restoreCell(row, column) {
    this.grid[row][column] = this.calculateCellBound(row, column);
  }

  point(row, column, anchor = Anchor.TL) {
    return this.cell(row, column).point(anchor);
  }

  bound() {
    return this.b.clone();
  }

  cell(row, column) {
    return this.grid[row][column];
  }

  setRowHeightRatio(row, ratio, anchor = Anchor.TL) {
    const ratioHeight = this.b.h * finiteNumber(ratio);
    const normalizedAnchor = normalizeAnchor(anchor);
    if ([Anchor.TL, Anchor.T, Anchor.TR].includes(normalizedAnchor)) {
      for (let column = 0; column < this.numColumns(); column += 1) {
        this.grid[row][column].resize(0, ratioHeight - this.grid[row][column].h, Anchor.TL);
      }
      return;
    }
    if ([Anchor.BL, Anchor.B, Anchor.BR].includes(normalizedAnchor)) {
      for (let column = 0; column < this.numColumns(); column += 1) {
        this.grid[row][column].resize(0, ratioHeight - this.grid[row][column].h, Anchor.BR);
      }
      return;
    }
    throw new Error("setRowHeightRatio supports top or base row anchors");
  }

  setColumnWidthRatio(anchor, column, ratio) {
    const ratioWidth = this.b.w * finiteNumber(ratio);
    const normalizedAnchor = normalizeAnchor(anchor);
    if ([Anchor.TL, Anchor.L, Anchor.BL].includes(normalizedAnchor)) {
      for (let row = 0; row < this.numRows(); row += 1) {
        this.grid[row][column].resize(ratioWidth - this.grid[row][column].w, 0, Anchor.TL);
      }
      return;
    }
    if ([Anchor.TR, Anchor.R, Anchor.BR].includes(normalizedAnchor)) {
      for (let row = 0; row < this.numRows(); row += 1) {
        this.grid[row][column].resize(ratioWidth - this.grid[row][column].w, 0, Anchor.BR);
      }
      return;
    }
    throw new Error("setColumnWidthRatio supports left or right column anchors");
  }

  numRows() {
    return this.grid.length;
  }

  numColumns() {
    return this.grid[0]?.length || 0;
  }
}
