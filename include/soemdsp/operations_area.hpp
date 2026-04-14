#pragma once
#include "soemdsp/sehelper.hpp"

#pragma warning(push)
#pragma warning(disable : 4458)


#include <vector>

namespace soemdsp::area {

/* NOMENCLATURE
* T/B/L/R/C Top / Base / Left / Right / Center
* W/H       Width / Height
* p/x/y     Point(x,y) / x-coord/y-coord
* i         Inner (non-padded / non-magin area)
* o         Outer (padded/margin area)
* m         Margin
* uw        unit width for grids
* uh        unit height for grids
* bound     x,y,w,h
* area      4 corner points on a square
* r/c       row / column
*/

enum class Anchor {
    TL, // Top Left
    T,  // Top Center or Top Side
    TR, // Top Right
    L,  // Mid Left or Left Side
    C,  // Mid Center
    R,  // Mid Right or Right Side
    BL, // Base Left
    B,  // Base Center or Base Side
    BR, // Base Right
};

template <typename value_t> 
struct Point {
    value_t x{}, y{};

    static Point center(const Point& p1, const Point& p2) {
        return { (p1.x + p2.x) * 0.5, (p1.y + p2.y)  * 0.5 };
    }

    static Point interpolate(const Point& p1, const Point& p2, double ratio) {
        return { p1.x + (p2.x - p1.x) * ratio, p1.y + (p2.y - p1.y) * ratio };
    }

    Point& move(value_t x, value_t y) {
        this->x += x;
        this->y += y;
        return *this;
    }
    Point& set(value_t x, value_t y) {
        move(x - this->x, y - this->y);
        return *this;
    }
};

template<typename value_t>
struct Line {
    using point_t = Point<value_t>;
    point_t start, end;

    // Returns the perpendicular distance from the line (start, end) to the point p.
    value_t getDistance(const point_t& p) const {
        // Compute the differences for the line vector.
        value_t dx = end.x - start.x;
        value_t dy = end.y - start.y;

        // Compute the numerator using the 2D cross product (or determinant).
        value_t numerator = std::abs(dy * p.x - dx * p.y + end.x * start.y - end.y * start.x); 

        // Compute the length of the line (denominator).
        value_t denominator = std::sqrt(dx * dx + dy * dy);

        // Avoid division by zero if the line is degenerate.
        if (denominator < std::numeric_limits<value_t>::epsilon()) {
            return {}; 
        }

        return numerator / denominator;
    }
};

template<typename value_t>
struct Bound {
    using point_t = Point<value_t>;
    value_t x{}, y{}, w{}, h{};

    Bound(point_t topLeft = { 0.0, 0.0 }, point_t baseRight = { 0.0, 0.0 }) {
        set(topLeft, baseRight);
    }
    Bound(point_t reference, value_t width = 0.0, value_t height = 0.0, Anchor anchor = Anchor::TL) {
        set(reference, width, height, anchor);
    }

    Bound& set(point_t topLeft, point_t baseRight) {
        x = topLeft.x;
        y = topLeft.y;
        w = abs(baseRight.x - topLeft.x);
        h = abs(baseRight.y - topLeft.y);
        return *this;
    }

    Bound& set(point_t reference, value_t width, value_t height, Anchor anchor = Anchor::TL) {
        w = abs(width);
        h = abs(height);
        switch (anchor) {
        case Anchor::TL:
            x = reference.x;
            y = reference.y;
            break;
        case Anchor::T:
            x = reference.x - w * 0.5;
            y = reference.y;
            break;
        case Anchor::TR:
            x = reference.x - w;
            y = reference.y;
            break;
        case Anchor::R:
            x = reference.x - w;
            y = reference.y - h * 0.5;
            break;
        case Anchor::BL:
            x = reference.x;
            y = reference.y - h;
            break;
        case Anchor::B:
            x = reference.x - w * 0.5;
            y = reference.y - h;
            break;
        case Anchor::BR:
            x = reference.x - w;
            y = reference.y - h;
            break;
        case Anchor::L:
            x = reference.x;
            y = reference.y - h * 0.5;
            break;
        case Anchor::C:
            x = reference.x - w * 0.5;
            y = reference.y - h * 0.5;
            break;
            SE_SWITCH_STATEMENT_FAILURE
        }
        return *this;
    }

    Bound& setWidth(value_t width, Anchor anchor) {
        adjustWidth(width - w, anchor);
        return *this;
    }
    Bound& setHeight(value_t height, Anchor anchor) {
        adjustHeight(height - h, anchor);
        return *this;
    }

    point_t point(Anchor anchor = Anchor::TL) const {
        switch (anchor) {
        case Anchor::TL:
            return { x, y };
        case Anchor::T:
            return { x + w * 0.5, y };
        case Anchor::TR:
            return { x + w, y };
        case Anchor::L:
            return { x, y + h * 0.5 };
        case Anchor::C:
            return { x + w * 0.5, y + h * 0.5 };
        case Anchor::R:
            return { x + w, y + h * 0.5 };
        case Anchor::BL:
            return { x, y + h };
        case Anchor::B:
            return { x + w * 0.5, y + h };
        case Anchor::BR:
            return { x + w, y + h };
            SE_SWITCH_STATEMENT_FAILURE
        }
        return { x, y };
    }

    // 0.0 being top/left, 1 being base/right, returns the top, base, left, or right area with no margin, *this keeps margin and reduces in size
    Bound splitPx(value_t pixels, Anchor anchor = Anchor::T) {
        switch (anchor) {
        case Anchor::T:
        case Anchor::B:
            return splitRatio(pixels / h, anchor);
        case Anchor::L:
        case Anchor::R:
            return splitRatio(pixels / w, anchor);
            SE_SWITCH_STATEMENT_FAILURE
        }
        return {};
    }
    Bound splitRatio(value_t ratio, Anchor anchor) {
        //SE_ASSERT(ratio >= 0.0 && ratio <= 1.0);

        Bound splitBound;

        switch (anchor) {
        case Anchor::T: 
            splitHorizontal(ratio, &splitBound, this);
            break;
        case Anchor::B: 
            splitHorizontal(ratio, this, &splitBound);
            break;        
        case Anchor::L: 
            splitVertical(ratio, &splitBound, this);
            break;
        case Anchor::R: 
            splitVertical(ratio, this, &splitBound);
            break;
            SE_SWITCH_STATEMENT_FAILURE
        }

        return std::move(splitBound);
    }

    Bound& move(value_t xAdjust, value_t yAdjust) {
        x += xAdjust;
        y += yAdjust;
        return *this;
    }

    // set width and height for the bound based on anchor
    Bound& resize(value_t width, value_t height, Anchor anchor = Anchor::TL) {
        if (width != 0) {
            adjustWidth(width, anchor);
        }
        if (height != 0) {
            adjustHeight(height, anchor);
        }
        return *this;
    }

    Bound& size(value_t width, value_t height, Anchor anchor = Anchor::TL) {
        resize(width - w, height - h, anchor);
        return *this;
    }
    Bound& square(Anchor anchor = Anchor::TL) {
        if (w > h) {
            adjustWidth(h - w, anchor);
        }
        else {
            adjustHeight(w - h, anchor);
        }
        return *this;
    }

  protected:
    void adjustWidth(value_t adjustment, Anchor anchor = Anchor::TL) {
        switch (anchor) {
        case Anchor::TR:
        case Anchor::R:
        case Anchor::BR:
            x -= adjustment;
            break;
        case Anchor::T:
        case Anchor::B:
        case Anchor::C:
            x -= adjustment * 0.5;
            break;
        case Anchor::TL:
        case Anchor::L:
        case Anchor::BL:
            break;
            SE_SWITCH_STATEMENT_FAILURE
        }
        w = abs(w + adjustment);
    }

    void adjustHeight(value_t adjustment, Anchor anchor = Anchor::TL) {
        switch (anchor) {
        case Anchor::BR:
        case Anchor::B:
        case Anchor::BL:
            y -= adjustment;
            break;
        case Anchor::R:
        case Anchor::L:
        case Anchor::C:
            y -= adjustment * 0.5;
            break;
        case Anchor::TL:
        case Anchor::T:
        case Anchor::TR:
            break;
            SE_SWITCH_STATEMENT_FAILURE
        }
        h = abs(h + adjustment);
    }
    // 0.0 being top, 1.0 being base, sets two given bounds into top and base based on split normalized position
    void splitHorizontal(value_t ratio, Bound* boundTop, Bound* boundBase) {
        point_t tl = point(Anchor::TL);
        if (ratio <= 0.0) {
            point_t tr = point(Anchor::TR);
            point_t br = point(Anchor::BR);
            boundTop->set(tl, tr);
            boundBase->set(tl, br);
        } else if (ratio >= 1.0) {
            point_t bl = point(Anchor::BL);
            point_t br = point(Anchor::BR);
            boundTop->set(tl, br);
            boundBase->set(bl, br);
        } else {
            point_t bl = point(Anchor::BL);
            point_t br = point(Anchor::BR);
            point_t p  = point_t::interpolate(tl, bl, ratio);
            boundBase->set(p, br);
            boundTop->set(tl, boundBase->point(Anchor::TR));
        }
    }

    // 0.0 being top, 1.0 being base, sets two given bounds into left and right based on split normalized position
    void splitVertical(value_t ratio, Bound* boundLeft, Bound* boundRight) {
        point_t tl = point(Anchor::TL);
        if (ratio <= 0.0) {
            point_t bl = point(Anchor::BL);
            point_t br = point(Anchor::BR);
            boundLeft->set(tl, bl);
            boundRight->set(tl, br);
        } else if (ratio >= 1.0) {
            point_t tr = point(Anchor::TR);
            point_t br = point(Anchor::BR);
            boundLeft->set(tl, br);
            boundRight->set(tr, br);
        } else {
            point_t bl = point(Anchor::BL);
            point_t br = point(Anchor::BR);
            point_t p  = point_t::interpolate(bl, br, ratio);
            boundLeft->set(tl, p);
            boundRight->set(boundLeft->point(Anchor::TR), br);
        }
    }
};


struct Grid {
    using bound_t = Bound<double>;
    using point_t = Point<double>;

    Grid(int rows, int columns, bound_t bound) {
        reset(rows, columns, bound.point(Anchor::TL), bound.point(Anchor::BR));
    }
    Grid(int rows, int columns, point_t topLeft, point_t baseRight) {
        reset(rows, columns, topLeft, baseRight);
    }
   
    void reset(int rows, int columns, point_t topLeft, point_t baseRight) {
        b.set(topLeft, baseRight);
        initCellWidth  = b.w / toDouble(columns);
        initCellHeight = b.h / toDouble(rows); 

        grid.resize(rows);
        for (size_t c = 0; c < rows; ++c) {
            grid[c].resize(columns);
        }

        double cellY{ b.y };       
        for (auto& row : grid) {
            double cellX{ b.x };
            for (auto& cell : row) {
                cell.set({ cellX, cellY }, initCellWidth, initCellHeight);
                cellX += initCellWidth;
            }
            cellY += initCellHeight;
        }
    }
 
    void sizeCells(double width, double height, Anchor anchor = Anchor::TL) {
        for (auto& r : grid) {
            for (auto& c : r) {
                c.size(width, height, anchor);
            }
        }
    }

    void squareCells(Anchor anchor = Anchor::TL) {
        for (auto& r : grid) {
            for (auto& c : r) {
                c.square(anchor);
            }
        }
    }

    
    bound_t calculateCellBound(int row, int column) {
        return { {initCellWidth * toDouble(row), initCellHeight * toDouble(column) }, initCellHeight, initCellHeight};
    }

    void restoreCell(int row, int column) {
        grid[toSizeT(row)][toSizeT(column)] = calculateCellBound(row, column);
    }

    point_t point(int row, int column, Anchor anchor = Anchor::TL) const {
        return grid[toSizeT(row)][toSizeT(column)].point(anchor);
    }

    bound_t bound() const {
        return b;
    }
    bound_t& cell(int row, int column) {
        return grid[toSizeT(row)][toSizeT(column)];
    }

    void setRowHeightRatio(int row, double ratio, Anchor anchor = Anchor::TL) {
        double ratioHeight = b.h * ratio;
        switch (anchor) {
        case Anchor::TL:
        case Anchor::T:
        case Anchor::TR:
            for (size_t i = 0; i < numColumns(); ++i) {
                grid[toSizeT(row)][i].resize(0.0, ratioHeight - grid[row][i].h, Anchor::TL);
            }
            break;
        case Anchor::BL:
        case Anchor::B:
        case Anchor::BR:
            for (size_t i = 0; i < numColumns(); ++i) {
                grid[toSizeT(row)][i].resize(0.0, ratioHeight - grid[row][i].h, Anchor::BR);
            }
            break;
        SE_SWITCH_STATEMENT_FAILURE
        }
    }

    void setColumnWidthRatio(Anchor anchor, int column, double ratio) {
        double ratioWidth = b.w * ratio;
        switch (anchor) {
        case Anchor::TL:
        case Anchor::L:
        case Anchor::BL:
            for (size_t i = 0; i < numRows(); ++i) {
                grid[i][column].resize(ratioWidth - grid[i][column].h, 0.0, Anchor::TL);
            }
            break;
        case Anchor::TR:
        case Anchor::R:
        case Anchor::BR:
            for (size_t i = 0; i < numRows(); ++i) {
                grid[i][toSizeT(column)].resize(ratioWidth - grid[i][column].h, 0.0, Anchor::BR);
            }
            break;
            SE_SWITCH_STATEMENT_FAILURE
        }
    }

    void setColumnWidthRatio(double /*column*/, double /*ratio*/) {
        //for (auto& row : grid) {
        //    for (auto& cell : row) {
        //    }
        //}
    }

    size_t numRows() {
        SE_ASSERT(grid.size());
        return grid.size();
    }
    size_t numColumns() {
        SE_ASSERT(grid.size());
        SE_ASSERT(grid[0].size());
        return grid[0].size();
    }

  protected:
    std::vector<std::vector<bound_t>> grid;
    bound_t b;
    double initCellWidth;
    double initCellHeight;
};

//
//struct Area {
//    Area(Point topLeft = { 0.0, 0.0 }, Point baseRight = { 0.0, 0.0 }) {
//        reset(topLeft, baseRight);
//    }
//    Area(Anchor anchor, Point reference, double width = 0.0, double height = 0) {
//        reset(anchor, reference, width, height);
//    }
//
//    void reset(Point topLeft, Point baseRight);
//    void reset(Anchor anchor, Point reference = { 0.0, 0.0 }, double width = 0.0, double height = 0);
//
//    void adjust(Anchor anchor, double width = 0.0, double height = 0.0, double topMargin = 0.0, double baseMargin = 0.0, double leftMargin = 0.0, double rightMargin = 0) {
//
//    }
//
//    void adjustWidth(Anchor anchor, double width);
//    void setWidth(Anchor anchor, double width);
//
//    void adjustHeight(Anchor anchor, double height);
//    void setHeight(Anchor anchor, double height);
//
//    void adjustMargin(Anchor anchor, double top, double base, double left, double right);
//    void setMargin(Anchor anchor, double top, double base, double left, double right);
//
//    // 0.0 being top, 1 being base, returns the base area with no margin, *this keeps margin and reduces in size
//    Area splitHorizontal(double ratio) {
//        SE_ASSERT(ratio >= 0.0 && ratio <= 1.0);
//        Area newArea(Point::interpolate(tl, bl, ratio), br);
//        adjustHeight(Anchor::T, -newArea.h);
//        return std::move(newArea);
//    }
//
//    // 0.0 being top, 1 being base, returns the right area with no margin, *this keeps margin and reduces in size
//    Area splitVertical(double ratio) {
//        SE_ASSERT(ratio >= 0.0 && ratio <= 1.0);
//        Area newArea(Point::interpolate(tl, tr, ratio), br);
//        adjustWidth(Anchor::L, -newArea.w);
//        return std::move(newArea);
//    }
//
//    Point point(Anchor anchor) {
//        switch (anchor) {
//        case Anchor::TL:
//            return tl;
//        case Anchor::T:
//            return t;
//        case Anchor::TR:
//            return tr;
//        case Anchor::L:
//            return l;
//        case Anchor::C:
//            return c;
//        case Anchor::R:
//            return r;
//        case Anchor::BL:
//            return bl;
//        case Anchor::B:
//            return b;
//        case Anchor::BR:
//            return br;
//            SE_SWITCH_STATEMENT_FAILURE
//        }
//    }
//
//    Point ow() {
//        return w;
//    }
//    Point oh() {
//        return h;
//    }
//    Point iw() {
//        return itr.x - itl.x;
//    }
//    Point ih() {
//        return ibl.y - itl.y;
//    }
//    Point itl() {
//        return itl;
//    }
//    Point itr() {
//        return itr;
//    }
//    Point ibl() {
//        return ibl;
//    }
//    Point ibr() {
//        return ibr;
//    }
//    Point ib() {
//        return Point::c(iBL_, ibr);
//    }
//    Point it() {
//        return Point::c(itl, itr);
//    }
//    Point il() {
//        return Point::c(itl, ibl);
//    }
//    Point ir() {
//        return Point::c(itr, ibr);
//    }
//    Point ic() {
//        return Point::c(itl, ibr);
//    }
//
//  protected:
//    Point itl, itr, ibl, ibr;
//    Point tl, t, tr, r, br, b, bl, l, c;
//    double w, h;
//};
//
//
//void Area::reset(Point topLeft, Point baseRight) {
//    w = baseRight.x - topLeft.x;
//    h = baseRight.y - topLeft.y;
//
//    itl = topLeft;
//    tl  = tl;
//    itr = { tl.x + w, tl.y };
//    tr  = itr;
//    ibr = baseRight;
//    br  = ibr;
//    ibl = { tl.x, tl.y + h };
//    bl  = ibl;
//    t   = Point::center(itl, itr);
//    r   = Point::center(itr, ibr);
//    b   = Point::center(ibl, ibr);
//    l   = Point::center(itl, ibl);
//    oC  = Point::center(itl, ibr);
//
//}
//
//
//void Area::reset(Anchor anchor, Point reference, double width, double height) {
//    Point topLeft, topRight, baseLeft, baseRight;
//    switch (anchor) {
//    case Anchor::TL:
//        topLeft     = { reference.x, reference.y };
//        baseRight = { topLeft.x + width, topLeft.x + height };
//        topRight    = { baseRight.x, topLeft.y };
//        baseLeft  = { topLeft.x, baseRight.y };
//        break;
//    case Anchor::T: {
//        double halfWidth{ width  * 0.5 };
//        topLeft     = { reference.x - halfWidth, reference.y };
//        baseRight = { topLeft.x + width, topLeft.x + height };
//        topRight    = { baseRight.x, topLeft.y };
//        baseLeft  = { topLeft.x, baseRight.y };
//        break;
//    }
//    case Anchor::TR:
//        topRight    = { reference.x, reference.y };
//        baseLeft  = { topRight.x - width, topRight.y + height };
//        topLeft     = { baseLeft.x, topRight.y };
//        baseRight = { topRight.x, baseLeft.y };
//        break;
//    case Anchor::R:
//        double halfHeight{ height  * 0.5 };
//        topRight    = { reference.x, reference.y - halfHeight };
//        baseLeft  = { topRight.x - width, topRight.y + height };
//        topLeft     = { baseLeft.x, topRight.y };
//        baseRight = { topRight.x, baseLeft.y };
//        break;
//    case Anchor::BR:
//        baseRight = { reference.x, reference.y };
//        topLeft     = { baseRight.x - width, baseRight.y - height };
//        baseLeft  = { topLeft.x, baseRight.y };
//        topRight    = { baseRight.x, topLeft.y };
//        break;
//    case Anchor::B:
//        double halfWidth{ width  * 0.5 };
//        baseRight = { reference.x + halfWidth, reference.y };
//        topLeft     = { baseRight.x - width, baseRight.y - height };
//        baseLeft  = { topLeft.x, baseRight.y };
//        topRight    = { baseRight.x, topLeft.y };
//        break;
//    case Anchor::BL:
//        baseLeft  = { reference.x, reference.y };
//        topRight    = { baseLeft.x + width, baseLeft.y - height };
//        baseRight = { topRight.x, baseLeft.y };
//        topLeft     = { baseLeft.x, topRight.y };
//        break;
//    case Anchor::L:
//        double halfHeight{ height  * 0.5 };
//        baseLeft  = { reference.x, reference.y + halfHeight };
//        topRight    = { baseLeft.x + width, baseLeft.y - height };
//        baseRight = { topRight.x, baseLeft.y };
//        topLeft     = { baseLeft.x, topRight.y };
//        break;
//    case Anchor::C:
//        double halfWidth{ width  * 0.5 };
//        double halfHeight{ height  * 0.5 };
//        topLeft     = { reference.x - halfWidth, reference.y - halfHeight };
//        baseRight = { reference.x + halfWidth, reference.y + halfHeight };
//        topRight    = { baseRight.x, topLeft.y };
//        baseLeft  = { topLeft.x, baseRight.y };
//        break;
//        SE_SWITCH_STATEMENT_FAILURE
//    }
//
//    reset(topLeft, topRight, baseLeft, baseRight);
//}
//
//
//void Area::adjustWidth(Anchor anchor, double adjustment) {
//    if (adjustment == 0) {
//        return;
//    }
//    w += adjustment;
//    double half{ adjustment * 0.5 };
//    switch (anchor) {
//    case Anchor::TL:
//    case Anchor::L:
//    case Anchor::BL:
//        itr.x += adjustment;
//        ibr.x += adjustment;
//        t.x += half;
//        tr.x += adjustment;
//        r.x += adjustment;
//        br.x += adjustment;
//        b.x += half;
//        c.x += half;
//        break;
//    case Anchor::T:
//    case Anchor::C:
//    case Anchor::B:
//        itl.x -= half;
//        itr.x += half;
//        ibr.x += half;
//        ibl.x -= half;
//        tl.x -= half;
//        tr.x += half;
//        br.x += half;
//        bl.x -= half;
//        r.x += half;
//        l.x -= half;
//        break;
//    case Anchor::TR:
//    case Anchor::R:
//    case Anchor::BR:
//        itl.x -= adjustment;
//        ibl.x -= adjustment;
//        t.x -= half;
//        tl.x -= adjustment;
//        l.x -= adjustment;
//        bl.x -= adjustment;
//        b.x -= half;
//        c.x -= half;
//        break;
//        SE_SWITCH_STATEMENT_FAILURE
//    }
//}
//
//void Area::adjustHeight(Anchor anchor, double adjustment) {
//    if (adjustment == 0) {
//        return;
//    }
//    h += adjustment;
//    double half{ adjustment * 0.5 };
//    switch (anchor) {
//    case Anchor::TL:
//    case Anchor::T:
//    case Anchor::TR:
//        ibl.y += adjustment;
//        ibr.y += adjustment;
//        l.y += half;
//        c.y += half;
//        r.y += half;
//        bl.y += adjustment;
//        b.y += adjustment;
//        br.y += adjustment;
//        break;
//    case Anchor::L:
//    case Anchor::C:
//    case Anchor::R:
//        itl.y -= half;
//        itr.y -= half;
//        ibr.y += half;
//        ibl.y += half;
//        tl.y -= half;
//        t.y -= half;
//        tr.y -= half;
//        bl.y += half;
//        b.y += half;
//        br.y += half;
//        break;
//    case Anchor::BL:
//    case Anchor::B:
//    case Anchor::BR:
//        itl.y -= adjustment;
//        itr.y -= adjustment;
//        l.y -= half;
//        c.y -= half;
//        r.y -= half;
//        tl.y -= adjustment;
//        t.y -= adjustment;
//        tr.y -= adjustment;
//        break;
//        SE_SWITCH_STATEMENT_FAILURE
//    }
//}
//
//void Area::setWidth(Anchor anchor, double width) {
//    double adjustment{ w - width };
//    adjustWidth(anchor, adjustment);
//}
//
//void Area::setHeight(Anchor anchor, double height) {
//    double adjustment{ h - height };
//    adjustHeight(anchor, adjustment);
//}
//
//
//void Area::setMargin(Anchor anchor, double top, double base, double left, double right) {
//    double topAdjust{ (itl.y - l.y) - top };
//    double rightAdjust{ (r.x - itr.x) - right };
//    double baseAdjust{ (b.y - ibl.y) - base };
//    double leftAdjust{ ibr.x - l.x };
//    adjustMargin(anchor, topAdjust, rightAdjust, baseAdjust, leftAdjust);
//}
//
//void Area::adjustMargin(Anchor anchor, double topAdjust, double baseAdjust, double leftAdjust, double rightAdjust) {
//    if (topAdjust != 0.0) {
//        double half = topAdjust  * 0.5;
//        switch (anchor) {
//        case Anchor::T:
//            itr.y += topAdjust;
//            itl.y += topAdjust;
//            ibl.y += topAdjust;
//            ibr.y += topAdjust;
//            l.y += half;
//            c.y += half;
//            r.y += half;
//            br.y += topAdjust;
//            b.y += topAdjust;
//            bl.y += topAdjust;
//            break;
//        case Anchor::L:
//        case Anchor::R:
//        case Anchor::B:
//            tl.y -= topAdjust;
//            t.y -= topAdjust;
//            tr.y -= topAdjust;
//            l.y -= half;
//            c.y -= half;
//            r.y -= half;
//            break;
//            SE_SWITCH_STATEMENT_FAILURE
//        }
//    }
//    if (rightAdjust != 0.0) {
//        double half = rightAdjust  * 0.5;
//        switch (anchor) {
//        case Anchor::R:
//            itl.x -= rightAdjust;
//            itr.x -= rightAdjust;
//            ibl.x -= rightAdjust;
//            ibr.x -= rightAdjust;
//            bl.x -= rightAdjust;
//            b.x -= rightAdjust;
//            br.x -= rightAdjust;
//            l.x -= rightAdjust;
//            c.x -= rightAdjust;
//            r.x -= rightAdjust;
//            break;
//        case Anchor::L:
//        case Anchor::T:
//        case Anchor::B:
//            t.x += half;
//            tr.x += rightAdjust;
//            c.x += half;
//            r.x += rightAdjust;
//            b.x += half;
//            br.x += rightAdjust;
//            break;
//            SE_SWITCH_STATEMENT_FAILURE
//        }
//    }
//    if (baseAdjust != 0.0) {
//        double half = baseAdjust  * 0.5;
//        switch (anchor) {
//        case Anchor::B:
//            itl.y -= baseAdjust;
//            itr.y -= baseAdjust;
//            ibl.y -= baseAdjust;
//            ibr.y -= baseAdjust;
//            tl.y -= baseAdjust;
//            t.y -= baseAdjust;
//            tr.y -= baseAdjust;
//            l.y -= half;
//            c.y -= half;
//            r.y -= half;
//            break;
//        case Anchor::L:
//        case Anchor::T:
//        case Anchor::R:
//            l.y += half;
//            c.y += half;
//            r.y += half;
//            bl.y += baseAdjust;
//            bl.y += baseAdjust;
//            br.y += baseAdjust;
//            break;
//            SE_SWITCH_STATEMENT_FAILURE
//        }
//    }
//    if (leftAdjust != 0.0) {
//        double half = leftAdjust  * 0.5;
//        switch (anchor) {
//        case Anchor::L:
//            itl.x += leftAdjust;
//            itr.x += leftAdjust;
//            ibl.x += leftAdjust;
//            ibr.x += leftAdjust;
//            t.x += half;
//            tr.x += leftAdjust;
//            c.x += half;
//            r.x += leftAdjust;
//            b.x += half;
//            br.x += leftAdjust;
//            break;
//        case Anchor::T:
//        case Anchor::R:
//        case Anchor::B:
//            tl.x -= leftAdjust;
//            l.x -= leftAdjust;
//            bl.x -= leftAdjust;
//            t.x -= half;
//            c.x -= half;
//            b.x -= half;
//            break;
//            SE_SWITCH_STATEMENT_FAILURE
//        }
//    }
//}
} // namespace soemdsp
#pragma warning(pop)
