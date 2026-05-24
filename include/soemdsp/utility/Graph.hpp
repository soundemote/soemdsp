#pragma once

#include <cfloat> // Preferred over <float.h> in modern C++
#include <cmath>
#include <vector>

#include <soemdsp/semath.hpp>
#include <soemdsp/utility/curve_functions.hpp>

namespace soemdsp::utility {

struct Graph {
    Graph() = default;
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

    void addNode(double x, double y, double c = 0.0, Node::Shape shape = Node::Shape::LINEAR) {
        nodes.push_back({ x, y, c, shape });
    }

    void moveNodeNoSort(int i, double x, double y) {
        nodes[static_cast<size_t>(i)].x = x;
        nodes[static_cast<size_t>(i)].y = y;
    }

    double getValueLinear(double x, int i) {
        double x1 = nodes[static_cast<size_t>(i)].x;
        double y1 = nodes[static_cast<size_t>(i)].y;
        double x2 = nodes[static_cast<size_t>(i + 1)].x;
        double y2 = nodes[static_cast<size_t>(i + 1)].y;

        if (std::abs(x2 - x1) < FLT_EPSILON) {
            return 0.5 * (y1 + y2);
        }

        return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
    }

    double getValueRational(double x, int i) {
        double x1 = nodes[static_cast<size_t>(i)].x;
        double y1 = nodes[static_cast<size_t>(i)].y;
        double x2 = nodes[static_cast<size_t>(i + 1)].x;
        double y2 = nodes[static_cast<size_t>(i + 1)].y;

        if (fabs(x2 - x1) < FLT_EPSILON) {
            return 0.5 * (y1 + y2);
        }

        double p = (x - x1) / (x2 - x1);
        return y1 + (y2 - y1) * curve::Rational{ nodes[static_cast<size_t>(i + 1)].c }.get(p);
    }

    double linVsExpFormulaScaler(double p) {
        const double c = 0.5 * (p + 1.0); // biplar to unipolar
        return 2.0 * log((1.0 - c) / c);
    }

    double getValueExponential(double x, int i) {
        double a = linVsExpFormulaScaler(nodes[static_cast<size_t>(i + 1)].c);

        const double x1 = nodes[static_cast<size_t>(i)].x;
        const double y1 = nodes[static_cast<size_t>(i)].y;
        const double x2 = nodes[static_cast<size_t>(i + 1)].x;
        const double y2 = nodes[static_cast<size_t>(i + 1)].y;

        if (fabs(x2 - x1) < std::numeric_limits<float>::epsilon()) {
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
        // return nodes[static_cast<size_t>(i-1)].y;

        switch (node->shape) {
        case Node::Shape::EXPONENTIAL:
            return getValueExponential(x, i);
        case Node::Shape::RATIONAL:
            return getValueRational(x, i);
        default: debug::FAIL();
        }
        return {};
    }

    // int firstIndexOfGreaterX(double xToFind)
    // {
    //     for (int i = 0; i < numNodesAdded; i++)
    //         if (nodes[static_cast<size_t>(i)].x > xToFind)
    //             return i;

    //     return numNodesAdded;
    // }
};
} // namespace soemdsp::utility
