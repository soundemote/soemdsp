#include "math.cxx"

class Node
{
	Node(){}
	
	Node(double xIn, double yIn, double curveIn)
	{
		x = xIn;
		y = yIn;
		curve = curveIn;
	}

	double x = 0;
	double y = 0;
	double curve = 0;
}

class MultipointGraph
{
	void addNode(double x, double y, double curve)
	{
		nodes.insertLast(Node(x, y, curve));
	}

	double getValue(double x)
	{
		if (nodes.length == 0)
			return 0;

		if (x < nodes[0].x)
			return nodes[0].y;

		uint i = firstIndexOfGreaterX(x);

		if (i == nodes.length)
			return nodes[i-1].y;

		return getValueRational(x, i-1);
	}

	protected uint firstIndexOfGreaterX(double xToFind)
	{
		for (uint i = 0; i < nodes.length; i++)
			if (nodes[i].x > xToFind)
				return i;

		return nodes.length;
	}

	protected double getValueRational(double x, uint i)
	{
		double x1 = nodes[i].x;
		double y1 = nodes[i].y;
		double x2 = nodes[i+1].x;
		double y2 = nodes[i+1].y;

		if (abs(x2-x1) < threshold)
			return 0.5 * (y1+y2);

		double p = (x-x1) / (x2-x1);
		return y1 + (y2-y1) * rationalCurve(p, nodes[i+1].curve);
	}

	array<Node> nodes;
	protected double threshold = 2.22045e-16;
}