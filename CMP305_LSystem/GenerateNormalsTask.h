#pragma once
#include "task.h"
#include "DXF.h"
#include "ExtraAlgorithms.h"

class GenerateNormalsTask : public Task
{
public:
	GenerateNormalsTask(XMFLOAT3* normal, const XMFLOAT3& pos_v1, const XMFLOAT3& pos_v2, const XMFLOAT3& pos_v3);
	~GenerateNormalsTask() override;

	void run() override;

private:
	//The normal location to be assigned
	XMFLOAT3* normal;
	//The three corner vertices
	XMFLOAT3 a, b, c;
};

