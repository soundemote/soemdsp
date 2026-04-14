#pragma once

#include <float.h>
#include <assert.h>
#include "../DBG.h"

#define MAX_GRAPH_NODES 50

enum GRAPH_SHAPES { LINEAR, RATIONAL, EXPONENTIAL };

typedef struct Node
{
    double x;
	double y;
	double c; // contour
    int shape;
} Node;

void Node_init(Node* o)
{
    o->x = 0;
    o->y = 0;
    o->c = 0;
    o->shape = RATIONAL;
}

typedef struct Graph
{
    int numNodesAdded;
    Node nodes[MAX_GRAPH_NODES];    
} Graph;

void Graph_init(Graph* o)
{
    o->numNodesAdded = 0;

    for (int i = 0; i < MAX_GRAPH_NODES; ++i)
        Node_init(&o->nodes[i]);    
}

void Graph_addNode(Graph* o, double x, double y, double c, int shape)
{
    assert(o->numNodesAdded < MAX_GRAPH_NODES);

    o->nodes[o->numNodesAdded].x = x;
    o->nodes[o->numNodesAdded].y = y;
    o->nodes[o->numNodesAdded].c = c;
    o->nodes[o->numNodesAdded].shape = shape;
    
    ++o->numNodesAdded;
}

void Graph_moveNodeNoSort(Graph* o, int i, double x, double y)
{
    assert(i < o->numNodesAdded);

    o->nodes[i].x = x;
    o->nodes[i].y = y;
}

double Graph_getValueLinear(Graph* o, double x, int i)
{
    double x1 = o->nodes[i].x;
    double y1 = o->nodes[i].y;
    double x2 = o->nodes[i + 1].x;
    double y2 = o->nodes[i + 1].y;

    if (fabs(x2 - x1) < FLT_EPSILON)
        return 0.5 * (y1 + y2);

    return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}

double Graph_getValueRational(Graph* o, double x, int i)
{
    double x1 = o->nodes[i].x;
    double y1 = o->nodes[i].y;
    double x2 = o->nodes[i + 1].x;
    double y2 = o->nodes[i + 1].y;

    if (fabs(x2 - x1) < FLT_EPSILON)
        return 0.5 * (y1 + y2);

    double p = (x - x1) / (x2 - x1);
    return y1 + (y2 - y1) * rationalCurve(p, o->nodes[i + 1].c);
}

double linVsExpFormulaScaler(double p)
{
    double c = 0.5 * (p + 1.0); // biplar to unipolar
    return 2.0 * log((1.0 - c) / c);
}

double Graph_getValueExponential(Graph* o, double x, int i)
{
    double a = linVsExpFormulaScaler(o->nodes[i + 1].shape);

    double x1 = o->nodes[i].x;
    double y1 = o->nodes[i].y;
    double x2 = o->nodes[i + 1].x;
    double y2 = o->nodes[i + 1].y;

    if (fabs(x2 - x1) < FLT_EPSILON)
        return 0.5 * (y1 + y2);

    double I = (x - x1) / (x2 - x1);
    return y1 + (y2 - y1) * (1.0 - exp(I * a)) / (1.0 - exp(a));
}

double Graph_getValue(Graph* o, double x)
{
    if (o->numNodesAdded == 0)
        return 0;

    if (x < o->nodes[0].x)
        return o->nodes[0].y;

    int i = Graph_firstIndexOfGreaterX(o, x);

    if (i == o->numNodesAdded)
        return o->nodes[i-1].y;

    switch(o->nodes[i].shape)
    {
    case EXPONENTIAL: return Graph_getValueExponential(o, x, i-1);
    case RATIONAL: return Graph_getValueRational(o, x, i-1);
    default: return Graph_getValueLinear(o, x, i-1);
    }
}

int Graph_firstIndexOfGreaterX(Graph* o, double xToFind)
{
    for (int i = 0; i < o->numNodesAdded; i++)
        if (o->nodes[i].x > xToFind)
            return i;

    return o->numNodesAdded;
}

