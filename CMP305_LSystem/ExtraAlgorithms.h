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

static int warp(int n, int lower, int upper)
{
	//warp n in the range [lower,upper]
	//for instance, with range [0,2], when n = 5, the function should return 1 (0,1,2,0,1)
	int current = lower;
	for (int i = 0; i < n; ++i)
	{
		++current;
		current = (current > upper) * lower + (current <= upper) * current;
	}
	return current;
}