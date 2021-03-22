#include <math.h>

#include "Point.h"

#define pi 3.141592653589793


static int Round(double x)
{
	if (x < 0.0f) return (int)(x - 0.5f);
	else return (int)(x + 0.5f);
}


void CPoint::Rotate(int wx, int wy, int wz, double dx, double dy, double dz)
{
	double cosinus, sinus, y2, x2;
	double fx = (double)x - dx;
	double fy = (double)y - dy;
	double fz = (double)z - dz;

	cosinus = cos((double)wx / (double)180.0 *(double)pi);
	sinus   = sin((double)wx / (double)180.0 *(double)pi);
	y2 = cosinus * fy - sinus * fz;
	fz = sinus * fy + cosinus * fz;
	fy = y2;

	cosinus = cos((double)wy / (double)180.0 * (double)pi);
	sinus   = sin((double)wy / (double)180.0 * (double)pi);
	x2 = cosinus * fx - sinus * fz;
	fz = sinus * fx + cosinus * fz;
    fx = x2;

	cosinus = cos((double)wz / (double)180.0 * (double)pi);
	sinus   = sin((double)wz / (double)180.0 * (double)pi);
	x2 = cosinus * fx - sinus * fy;
	fy = sinus * fx + cosinus * fy;
	fx = x2;

	x = Round(fx + dx);
	y = Round(fy + dy);
	z = Round(fz + dz);
}