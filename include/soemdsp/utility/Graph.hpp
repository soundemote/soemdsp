#pragma once

#include "soemdsp/semath.hpp"
#include "soemdsp/curve_functions.hpp"

#include <cmath>
#include <float.h>
#include <vector>

namespace soemdsp::utility {

class Graph {
    struct Node {
        enum class Shape {
            LINEAR,
            RATIONAL,
            EXPONENTIAL
        };
        double x{};
        double y{};
        double c{}; // contour
        Shape shape{ Shape::RATIONAL };
    };

    std::vector<Node> nodes{};

    void addNode(double x, double y, double c, Node::Shape shape) {
        nodes.push_back({ x, y, c, shape });
    }

    void moveNodeNoSort(int i, double x, double y) {
        nodes[toSizeT(i)].x = x;
        nodes[toSizeT(i)].y = y;
    }

    double getValueLinear(double x, int i) {
        double x1 = nodes[toSizeT(i)].x;
        double y1 = nodes[toSizeT(i)].y;
        double x2 = nodes[toSizeT(i + 1)].x;
        double y2 = nodes[toSizeT(i + 1)].y;

        if (std::abs(x2 - x1) < FLT_EPSILON)
            return 0.5 * (y1 + y2);

        return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
    }

    double getValueRational(double x, int i) {
        double x1 = nodes[toSizeT(i)].x;
        double y1 = nodes[toSizeT(i)].y;
        double x2 = nodes[toSizeT(i + 1)].x;
        double y2 = nodes[toSizeT(i + 1)].y;

        if (fabs(x2 - x1) < FLT_EPSILON)
            return 0.5 * (y1 + y2);

        double p = (x - x1) / (x2 - x1);
        return y1 + (y2 - y1) * curve::Rational{ nodes[toSizeT(i + 1)].c }.get(p);
    }

    double linVsExpFormulaScaler(double p) {
        const double c = 0.5 * (p + 1.0); // biplar to unipolar
        return 2.0 * log((1.0 - c) / c);
    }

    double getValueExponential(double x, int i) {
        double a = linVsExpFormulaScaler(nodes[toSizeT(i + 1)].c);

        const double x1 = nodes[toSizeT(i)].x;
        const double y1 = nodes[toSizeT(i)].y;
        const double x2 = nodes[toSizeT(i + 1)].x;
        const double y2 = nodes[toSizeT(i + 1)].y;

        if (fabs(x2 - x1) < FLT_EPSILON) {
            return 0.5 * (y1 + y2);
        }

        const double I = (x - x1) / (x2 - x1);
        return y1 + (y2 - y1) * (1.0 - exp(I * a)) / (1.0 - exp(a));
    }

    double getValue(double x) {
        using std::begin;
        using std::distance;
        using std::end;
        using std::find_if;
        using std::rbegin;
        using std::vector;

        if (nodes.empty()) {
            return 0;
        }

        if (x < nodes[0].x) {
            return nodes[0].y;
        }

        auto node = find_if(begin(nodes), end(nodes), [x](const Node& item) { return item.x > x; });
        int i     = static_cast<int>(distance(begin(nodes), node));

        if (node == end(nodes)) {
            return rbegin(nodes)->y;
        }

        // if (i == nodes.size())
        // return nodes[toSizeT(i-1)].y;

        switch (node->shape) {
        case Node::Shape::EXPONENTIAL:
            return getValueExponential(x, i);
        case Node::Shape::RATIONAL:
            return getValueRational(x, i);
            SE_SWITCH_STATEMENT_FAILURE
        }
        return {};
    }

    // int firstIndexOfGreaterX(double xToFind)
    // {
    //     for (int i = 0; i < numNodesAdded; i++)
    //         if (nodes[toSizeT(i)].x > xToFind)
    //             return i;

    //     return numNodesAdded;
    // }
};
} // namespace soemdsp::utility
