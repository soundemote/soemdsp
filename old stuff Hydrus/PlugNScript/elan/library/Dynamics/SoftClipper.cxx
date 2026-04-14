/*
Taken from RS-MET library, this is a soft clipper.
*/
class SoftClipper
{
	// Sets the center value. This is the point where x = y = center (assuming the prototype 
	// function goes through the origin).
	void setCenter(double v)
	{
		center = v;
		updateCoeffs();
	}

	// Sets the width. The output values of the function will be bounded by center +- width/2. 
	// newWidth should be > 0.
	void setWidth(double v)
	{
		width = v;
		updateCoeffs();
	}

	// Computes the output value for the given input x.
	double getSample(double v)
	{
		return shiftY + scaleY * tanh(scaleX * v + shiftX);
	}

	// Updates the coefficients from the desired center and width.
	protected void updateCoeffs()
	{
		scaleX =  2 / width;
		shiftX = -1 - (scaleX * (center - 0.5 * width));
		scaleY =  1 / scaleX;
		shiftY = -shiftX * scaleY;
	}

	double center = 0;
	double width  = 2;
	double scaleX = 1, scaleY = 1;  // scale factors for input and output
	double shiftX = 0, shiftY = 0;  // shift offsets for input and output
};