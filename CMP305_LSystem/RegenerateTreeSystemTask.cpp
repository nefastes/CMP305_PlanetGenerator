#include "RegenerateTreeSystemTask.h"

RegenerateTreeSystemTask::RegenerateTreeSystemTask(ID3D11Device* d, ID3D11DeviceContext* dc, Tree* tree_to_modify) : device_(d), device_context_(dc),
tree_to_modify_(tree_to_modify)
{
}

RegenerateTreeSystemTask::~RegenerateTreeSystemTask()
{
	device_ = nullptr;
	device_context_ = nullptr;
	tree_to_modify_ = nullptr;
}

void RegenerateTreeSystemTask::run()
{
	tree_to_modify_->runSystem();
	tree_to_modify_->build(device_, device_context_);
}
