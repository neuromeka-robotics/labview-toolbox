
#include "stdafx.h"
#include "Utils.h"

int count = 0;

double GetSA(double radius)
{
	return 4 * M_PI * radius * radius;
}

double GetVol(double radius)
{
	return 4.0 / 3.0 * M_PI * pow(radius, 3.0);
}

