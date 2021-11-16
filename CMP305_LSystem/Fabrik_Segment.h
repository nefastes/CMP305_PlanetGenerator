#pragma once

#include "DXF.h"

class Fabrik_Segment
{
public:
	Fabrik_Segment(XMFLOAT3 goal, XMFLOAT3 pivot, XMFLOAT3 root);
	~Fabrik_Segment();

	void update(XMFLOAT3& goal);

	const XMFLOAT3& getRootPos() { return root_; }
	const XMFLOAT3& getPivotPos() { return pivot_; }
	const XMFLOAT3& getGoal() { return goal_; }
	const XMMATRIX& getTransform() { return transform_; }

private:
	XMFLOAT3 root_;		//position of the root of the segment
	XMFLOAT3 pivot_;	//position of the pivot of the segment
	XMFLOAT3 goal_;		//position of its goal
	XMMATRIX transform_;
};

