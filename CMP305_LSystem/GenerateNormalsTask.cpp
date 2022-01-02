#include "GenerateNormalsTask.h"

GenerateNormalsTask::GenerateNormalsTask(XMFLOAT3* normal, const XMFLOAT3& pos_v1, const XMFLOAT3& pos_v2, const XMFLOAT3& pos_v3)
{
	this->normal = normal;
	a = pos_v1;
	b = pos_v2;
	c = pos_v3;
}

GenerateNormalsTask::~GenerateNormalsTask()
{
	normal = nullptr;
}

void GenerateNormalsTask::run()
{
	//Calculate the plane normals
	XMFLOAT3 cross;		//Cross product result
	float mag;			//Magnitude of the cross product (so we can normalize it)
	XMFLOAT3 ab;		//Edge 1
	XMFLOAT3 ac;		//Edge 2

	//Two edges
	ab = XMFLOAT3(c.x - a.x, c.y - a.y, c.z - a.z);
	ac = XMFLOAT3(b.x - a.x, b.y - a.y, b.z - a.z);

	//Calculate the cross product
	cross.x = ab.y * ac.z - ab.z * ac.y;
	cross.y = ab.z * ac.x - ab.x * ac.z;
	cross.z = ab.x * ac.y - ab.y * ac.x;
	mag = (cross.x * cross.x) + (cross.y * cross.y) + (cross.z * cross.z);
	mag = sqrtf(mag);
	cross.x /= mag;
	cross.y /= mag;
	cross.z /= mag;
	*normal = cross;
}
