class Rotator3D
{
	Rotator3D(double angleX = 0, double angleY = 0, double angleZ = 0)
	{
		rx = angleX*TAU;
		ry = angleY*TAU;
		rz = angleZ*TAU;
		updateCoeffs();
	}

	void setAngleX(double X)
	{
		rx = X*TAU;
		updateCoeffs();
	}

	void setAngleY(double Y)
	{
		ry = Y*TAU;
		updateCoeffs();
	}

	void setAngleZ(double Z)
	{
		rz = Z*TAU;
		updateCoeffs();
	}

	void setAngles(double X, double Y, double Z)
	{
		rx = X*TAU;
		ry = Y*TAU;
		rz = Z*TAU;
		updateCoeffs();
	}

	/** Applies the rotation matrix to the coordinate values. */
	void apply(double& x, double& y, double& z)
	{
		// temporaries:
		double X = x; 
		double Y = y; 
		double Z = z;
		
		// new vector is given by matrix-vector product:
		x = xx*X + xy*Y + xz*Z;    // |x|   |xx xy xz|   |X|
		y = yx*X + yy*Y + yz*Z;    // |y| = |yx yy yz| * |Y|
		z = zx*X + zy*Y + zz*Z;    // |z|   |zx zy zz|   |Z|
	}

	/** Updates the rotation matrix coefficients. */
	protected void updateCoeffs()
	{
		// sines/cosines:
		double sx = sin(rx); double cx = cos(rx);
		double sy = sin(ry); double cy = cos(ry);
		double sz = sin(rz); double cz = cos(rz);
		
		// rotation matrix coeffs:
		xx =  cz*cy;
		xy = -sz*cx + cz*sy*sx;
		xz =  sz*sx + cz*sy*cx;
		yx =  sz*cy;
		yy =  cz*cx + sz*sy*sx;
		yz = -cz*sx + sz*sy*cx;
		zx = -sy;
		zy =  cy*sx;
		zz =  cy*cx;
	}

	double rx, ry, rz; // rotation angles around x, y and z-axis
	double xx, xy, xz, yx, yy, yz, zx, zy, zz; // matrix coeffs
};