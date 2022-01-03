#pragma once
#include "task.h"
#include "DXF.h"
#include "Tree.h"

class RegenerateTreeSystemTask : public Task
{
public:
	RegenerateTreeSystemTask(ID3D11Device* d, ID3D11DeviceContext* dc, Tree* tree_to_modify);
	~RegenerateTreeSystemTask() override;

	void run() override;

private:
	ID3D11Device* device_;
	ID3D11DeviceContext* device_context_;
	Tree* tree_to_modify_;
};

