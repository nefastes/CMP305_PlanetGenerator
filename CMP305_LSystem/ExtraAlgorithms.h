#pragma once
static float clamp(float n, float lower, float upper)
{
	//if n < lower return lower
	//if n > upper return upper
	//else return n
	bool check1 = n < lower;
	bool check2 = n > upper;
	return check1 * lower + check2 * upper + (!check1 && !check2) * n;
};