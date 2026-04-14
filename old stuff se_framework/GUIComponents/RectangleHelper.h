#pragma once

enum Anchor
{
	TopLeft,
	TopCenter,
	TopRight,
	MidLeft,
	MidCenter,
	MidRight,
	BottomLeft,
	BottomCenter,
	BottomRight,
	None
};

enum Edge
{
	Left,
	Top,
	Right,
	Bottom,
};

struct ElanRect
{
	// Get a rectangle point based on anchor.
	template <typename T> static juce::Point<T> getPoint(const juce::Rectangle<T>& object, Anchor anchor)
	{
		switch (anchor)
		{
		case Anchor::TopLeft:
			return object.getTopLeft();
		case Anchor::TopCenter:
			return { object.getCentreX(), object.getY() };
		case Anchor::TopRight:
			return object.getTopRight();
		case Anchor::MidLeft:
			return { object.getX(), object.getCentreY() };
		case Anchor::MidCenter:
			return object.getCentre();
		case Anchor::MidRight:
			return { object.getRight(), object.getCentreY() };
		case Anchor::BottomLeft:
			return object.getBottomLeft();
		case Anchor::BottomCenter:
			return { object.getCentreX(), object.getBottom() };
		case Anchor::BottomRight:
			return object.getBottomRight();
		case Anchor::None:
		default:
			jassertfalse;
			return {};
		}
	}

	// Place a rectangle based on a given point and anchor of given rectangle.
	template <typename T> static juce::Rectangle<T> place(const juce::Rectangle<T>& object, const juce::Point<T>& subject, Anchor objectAnchor = Anchor::None)
	{
		switch (objectAnchor)
		{
		case Anchor::TopLeft:
			return { subject.getX(), subject.getY(), object.getWidth(), object.getHeight() };
		case Anchor::TopCenter:
			return { subject.getX() - object.getWidth() / T(2), subject.getY(), object.getWidth(), object.getHeight() };
		case Anchor::TopRight:
			return { subject.getX() - object.getWidth(), subject.getY(), object.getWidth(), object.getHeight() };
		case Anchor::MidLeft:
			return { subject.getX(), subject.getY() - object.getHeight() / T(2), object.getWidth(), object.getHeight() };
		case Anchor::MidCenter:
			return { subject.getX() - object.getWidth() / T(2), subject.getY() - object.getHeight() / T(2), object.getWidth(), object.getHeight() };
		case Anchor::MidRight:
			return { subject.getX() - object.getWidth(), subject.getY() - object.getHeight() / T(2), object.getWidth(), object.getHeight() };
		case Anchor::BottomLeft:
			return { subject.getX(), subject.getY() - object.getHeight(), object.getWidth(), object.getHeight() };
		case Anchor::BottomCenter:
			return { subject.getX() - object.getWidth() / T(2), subject.getY() - object.getHeight(), object.getWidth(), object.getHeight() };
		case Anchor::BottomRight:
			return { subject.getX() - object.getWidth(), subject.getY() - object.getHeight(), object.getWidth(), object.getHeight() };
		case Anchor::None:
		default:
			jassertfalse;
			return {};
		}
	}

	// Place a rectangle next to another rectangle based on anchor and points.
	template <typename T> static juce::Rectangle<T> place(const juce::Rectangle<T> & object, const juce::Rectangle<T> & subject, Anchor objectAnchor = Anchor::None, Anchor subjectAnchor = Anchor::None)
	{
		juce::Point<T> p;

		switch (subjectAnchor)
		{
		case None:
			p = getPoint(object, TopLeft);
			break;
		case Anchor::TopLeft:
		case Anchor::TopCenter:
		case Anchor::TopRight:
		case Anchor::MidLeft:
		case Anchor::MidCenter:
		case Anchor::MidRight:
		case Anchor::BottomLeft:
		case Anchor::BottomCenter:
		case Anchor::BottomRight:
			p = getPoint(subject, subjectAnchor);
			break;
		default:
			jassertfalse;
		}

		switch (objectAnchor)
		{
		case Anchor::None:
			[[fallthrough]];
		case Anchor::TopLeft:
			return { p.getX(), p.getY(), object.getWidth(), object.getHeight() };
		case Anchor::TopCenter:
			return { p.getX() - object.getWidth() / T(2), p.getY(), object.getWidth(), object.getHeight() };
		case Anchor::TopRight:
			return { p.getX() - object.getWidth(), p.getY(), object.getWidth(), object.getHeight() };
		case Anchor::MidLeft:
			return { p.getX(), p.getY() - object.getHeight() / T(2), object.getWidth(), object.getHeight() };
		case Anchor::MidCenter:
			return { p.getX() - object.getWidth() / T(2), p.getY() - object.getHeight() / T(2), object.getWidth(), object.getHeight() };
		case Anchor::MidRight:
			return { p.getX() - object.getWidth(), p.getY() - object.getHeight() / T(2), object.getWidth(), object.getHeight() };
		case Anchor::BottomLeft:
			return { p.getX(), p.getY() - object.getHeight(), object.getWidth(), object.getHeight() };
		case Anchor::BottomCenter:
			return { p.getX() - object.getWidth() / T(2), p.getY() - object.getHeight(), object.getWidth(), object.getHeight() };
		case Anchor::BottomRight:
			return { p.getX() - object.getWidth(), p.getY() - object.getHeight(), object.getWidth(), object.getHeight() };
		default:
			jassertfalse;
			return {};
		}
	}

	template <typename T> static juce::Rectangle<T> fit(const juce::Rectangle<T> & object, const juce::Rectangle<T> & subject, Anchor objectAnchor = Anchor::None, Anchor subjectAnchor = Anchor::None)
	{
		auto deltaWidth = subject.getWidth() - object.getWidth();
		auto deltaHeight = subject.getHeight() - object.getHeight();

		auto ratio = object.getAspectRatio();

		//juce::Rectangle<float> r;
		//if (deltaWidth < deltaHeight)
		//	r = { object.getWidth() + deltaWidth * ratio, subject.getHeight() };
		//else
		//	r = { subject.getWidth(), object.getHeight() + deltaHeight * ratio };

		juce::Rectangle<float> r;
		if (deltaWidth < deltaHeight)
			r = { subject.getWidth(), object.getHeight() + deltaWidth * ratio };
		else
			r = { object.getWidth() + deltaHeight * ratio, subject.getHeight() };

		return place(r, subject, objectAnchor, subjectAnchor);
	}

	template <typename T> static juce::Rectangle<T> pad(const juce::Rectangle<T> & object, T marginLeft, T marginTop, T marginRight, T marginBottom, Anchor anchor = Anchor::None)
	{
		T x = object.getX() + marginLeft;
		T y = object.getY() + marginTop;
		T w = object.getWidth() - (marginLeft + marginRight);
		T h = object.getHeight() - (marginTop + marginBottom);

		return place({ x, y, w, h }, object, anchor, anchor);
	}

	template <typename T> static juce::Rectangle<T> pad(const juce::Rectangle<T> & object, T margin, Edge edge, Anchor anchor = Anchor::None)
	{
		switch (edge)
		{
		case Left:
			return pad(object, margin, T(0), T(0), T(0), anchor);
		case Top:
			return pad(object, T(0), margin, T(0), T(0), anchor);
		case Right:
			return pad(object, T(0), T(0), margin, T(0), anchor);
		case Bottom:
			return pad(object, T(0), T(0), T(0), margin, anchor);
		default:
			jassertfalse;
			return {};
		}		
	}
	
	template <typename T> static juce::Rectangle<T> resize(const juce::Rectangle<T> & object, T delta, Anchor anchor = Anchor::None)
	{
		return place({object.getX(), object.getY(), object.getWidth() + delta, object.getHeight() + delta}, object, anchor, anchor);
	}

	template <typename T> static juce::Rectangle<T> resize(const juce::Rectangle<T> & object, T deltaWidth, T deltaHeight, Anchor anchor = Anchor::None)
	{
		return place({ object.getWidth() + deltaWidth, object.getHeight() + deltaHeight }, object, anchor, anchor);
	}

	template <typename T> static juce::Rectangle<T> resize(const juce::Rectangle<T> & object, T delta, Edge edge)
	{
		switch (edge)
		{
		case Left:
			return place({ object.getWidth() + delta, object.getHeight() }, object, Anchor::MidRight, Anchor::MidRight);
		case Top:
			return place({ object.getWidth(), object.getHeight() + delta }, object, Anchor::BottomCenter, Anchor::BottomCenter);
		case Right:
			return place({ object.getWidth() + delta, object.getHeight() }, object, Anchor::MidLeft, Anchor::MidLeft);
		case Bottom:
			return place({ object.getWidth(), object.getHeight() + delta }, object, Anchor::TopCenter, Anchor::TopCenter);
		default:
			jassertfalse;
			return {};
		}
	}

	template <typename T> static juce::Rectangle<T> setSize(const juce::Rectangle<T> & object, T width, T height, Anchor anchor = Anchor::None)
	{
		return place({ width, height }, object, anchor, anchor);
	}

	template <typename T> static juce::Rectangle<T> setSize(const juce::Rectangle<T> & object, T squareSize, Anchor anchor = Anchor::None)
	{
		return place({ squareSize, squareSize }, object, anchor, anchor);
	}

	template <typename T> static juce::Rectangle<T> setWidth(const juce::Rectangle<T> & object, T width, Anchor anchor = Anchor::None)
	{
		return place({ width, object.getHeight() }, object, anchor, anchor);
	}

	template <typename T> static juce::Rectangle<T> setHeight(const juce::Rectangle<T> & object, T height, Anchor anchor = Anchor::None)
	{
		return place({ object.getWidth(), height }, object, anchor, anchor);
	}

	template <typename T> static juce::Rectangle<T> scale(const juce::Rectangle<T> & object, T scale, Anchor anchor = Anchor::None)
	{
		return place({ object.getWidth() * scale, object.getHeight() * scale }, object, anchor, anchor);
	}

	/*
	Get a rectangle for the space between area1 and area2;
	*/
	template <typename T> static juce::Rectangle<T> space(const juce::Rectangle<T> & subject1, const juce::Rectangle<T> & subject2)
	{
		return { subject1.getBottomLeft(), subject2.getTopRight() };
	}
};

struct ElanPath
{
	static Path createPathFromPoints(const vector<juce::Point<float>> & v)
	{
		Path p;

		p.startNewSubPath(v[0]);

		for (size_t i = 1; i < v.size(); ++i)
			p.lineTo(v[i]);

		p.closeSubPath();

		return p;
	}

	static Path plus(juce::Rectangle<float> bounds, float thickness)
	{
		float x = bounds.getX();
		float y = bounds.getY();
		float w = bounds.getWidth();
		float h = bounds.getHeight();
		float wHalf = bounds.getWidth() * 0.5f;
		float hHalf = bounds.getHeight() * 0.5f;
		float tHalf = thickness * 0.5f;

		vector<juce::Point<float>> points
		{
			{x + wHalf - tHalf, y},
			{x + wHalf + tHalf, y},

			{x + wHalf + tHalf, y + hHalf - tHalf},

			{x + w, y + hHalf - tHalf},
			{x + w, y + hHalf + tHalf},

			{x + wHalf + tHalf, y + hHalf + tHalf},

			{x + wHalf + tHalf, y + h},
			{x + wHalf - tHalf, y + h},

			{x + wHalf - tHalf, y + hHalf + tHalf},

			{x, y + hHalf + tHalf},
			{x, y + hHalf - tHalf},

			{x + wHalf - tHalf, y + hHalf - tHalf}
		};

		return createPathFromPoints(points);
	}

	static Path corner(juce::Rectangle<float> bounds, float thickness, Anchor anchor)
	{
		float x = bounds.getX();
		float y = bounds.getY();
		float w = bounds.getWidth();
		float h = bounds.getHeight();

		vector<juce::Point<float>> points;

		switch (anchor)
		{
		case TopLeft:
			points = {
				{x, y},
				{x + w, y},
				{x + w, y + thickness},
				{x + thickness, y + thickness},
				{x + thickness, y + h},
				{x, y + h},
			};
			break;
		case TopRight:
			points = {
				{x, y},
				{x + w, y},
				{x + w, y + h},
				{x + w - thickness, y + h},
				{x + w - thickness, y + thickness},
				{x, y + thickness},				
			};
			break;
		case BottomRight:
			points = {
				{x + w - thickness, y},
				{x + w, y},
				{x + w, y + h},
				{x, y + h},
				{x, y + h - thickness},
				{x + w - thickness, y + h - thickness}
			};
			break;
		case BottomLeft:
			points = {
				{x, y},
				{x + thickness, y},
				{x + thickness, y + h - thickness},
				{x + w, y + h - thickness},
				{x + w, y + h},
				{x, y + h}
			};
			break;
		default:
			jassertfalse;
		}

		return createPathFromPoints(points);
	}

	static Path v(juce::Rectangle<float> bounds, float thickness, Edge edgeToPoint)
	{
		float x = bounds.getX();
		float y = bounds.getY();
		float w = bounds.getWidth();
		float h = bounds.getHeight();

		float wHalf = bounds.getWidth() * 0.5f;

		vector<juce::Point<float>> points;

		switch (edgeToPoint)
		{
		case Left:
			jassertfalse;
		case Top:
			jassertfalse;
		case Right:
			jassertfalse;
		case Bottom:
			points = {
				{x, y},
			  {x + wHalf, y + h},
				{x + w, y},
				{x + w - thickness, y},
				{x + wHalf, y + h - thickness},
				{x + thickness, y},
			};
			break;
		default:
			jassertfalse;
		}

		return createPathFromPoints(points);
	}

	static Path triangle(juce::Rectangle<float> bounds, Edge edgeToPoint)
	{
		float x = bounds.getX();
		float y = bounds.getY();
		float w = bounds.getWidth();
		float h = bounds.getHeight();

		float hHalf = bounds.getHeight() * 0.5f;

		vector<juce::Point<float>> points;

		switch (edgeToPoint)
		{
		case Left:
			points = {
				{x + w, y},
				{x + w, y + h},
				{x, y + hHalf},
			};
			break;
		case Top:
			jassertfalse;
		case Right:
			points = {
				{x, y},
				{x, y + h},
				{x + w, y + hHalf},
			};
		case Bottom:
			break;
		default:
			jassertfalse;
		}

		return createPathFromPoints(points);
	}

	static Path threeSides(juce::Rectangle<float> bounds, float thickness, Edge openSide)
	{
		float x = bounds.getX();
		float y = bounds.getY();
		float w = bounds.getWidth();
		float h = bounds.getHeight();
		float t = thickness;

		vector<juce::Point<float>> points;

		switch (openSide)
		{
		case Left:
			points = {
				{x, y},
				{x + w, y},
				{x + w, y + h},
				{x, y + h},
				{x, y + h - t},
				{x + w - t, y + h - t},
				{x + w - t, y + t},
				{x, y + t},
			};
			break;
		case Top:
			jassertfalse;
			break;
		case Right:
			jassertfalse;
			break;
		case Bottom:
			break;
		default:
			jassertfalse;
		}

		return createPathFromPoints(points);
	}

	static Path burgerMenu(juce::Rectangle<float> bounds, float barThickness)
	{
		Path p;

		auto area = bounds;

		auto barSpacing = (bounds.getHeight() - barThickness * 3.f) / 2.f;

		p.addRectangle(area.removeFromTop(barThickness));
		area.removeFromTop(barSpacing);
		p.addRectangle(area.removeFromTop(barThickness));
		area.removeFromTop(barSpacing);
		p.addRectangle(area.removeFromTop(barThickness));

		return p;
	}
};
