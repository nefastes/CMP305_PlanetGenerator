#include "Fabrik_Segment.h"

Fabrik_Segment::Fabrik_Segment(XMFLOAT3 goal, XMFLOAT3 pivot, XMFLOAT3 root)
{
	goal_ = goal;
	pivot_ = pivot;
	root_ = root;
	transform_ = XMMatrixIdentity();
}

Fabrik_Segment::~Fabrik_Segment()
{
}

void Fabrik_Segment::update(XMFLOAT3& goal)
{
}
