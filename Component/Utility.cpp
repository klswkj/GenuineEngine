#include "stdafx.h"
#include "Utility.h"

XMFLOAT4X4 aiMatrixToXMFloat4x4(const aiMatrix4x4* aiMe)
{
	auto offset = *aiMe;
	XMFLOAT4X4 output;
	//auto mat = XMMatrixTranspose(XMMATRIX(&aiMe->a1));
	XMMATRIX mat = XMMatrixTranspose(
		XMMATRIX(offset.a1, offset.a2, offset.a3, offset.a4,
			offset.b1, offset.b2, offset.b3, offset.b4,
			offset.c1, offset.c2, offset.c3, offset.c4,
			offset.d1, offset.d2, offset.d3, offset.d4));
	XMStoreFloat4x4(&output, mat);
	return output;
}

XMFLOAT3X3 aiMatrixToXMFloat3x3(const aiMatrix3x3 * aiMe)
{
	XMFLOAT3X3 output;
	output._11 = aiMe->a1;
	output._12 = aiMe->a2;
	output._13 = aiMe->a3;

	output._21 = aiMe->b1;
	output._22 = aiMe->b2;
	output._23 = aiMe->b3;

	output._31 = aiMe->c1;
	output._32 = aiMe->c2;
	output._33 = aiMe->c3;

	return output;
}


XMMATRIX OGLtoXM(const ogldev::Matrix4f& mat)
{
	auto xm = XMMATRIX(
		mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
		mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
		mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
		mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]
	);

	return xm;
}

bool IsIntersecting(DirectX::BoundingOrientedBox boundingBox, Camera* camera, int mouseX, int mouseY, float& distance)
{
	uint16_t screenWidth = 1280;
	uint16_t screenHeight = 720;
	auto viewMatrix = XMLoadFloat4x4(&camera->GetViewMatrix());
	auto projMatrix = XMLoadFloat4x4(&camera->GetProjectionMatrix());

	auto orig = XMVector3Unproject(XMVectorSet((float)mouseX, (float)mouseY, 0.f, 0.f),
		0,
		0,
		screenWidth,
		screenHeight,
		0,
		1,
		projMatrix,
		viewMatrix,
		XMMatrixIdentity());

	auto dest = XMVector3Unproject(XMVectorSet((float)mouseX, (float)mouseY, 1.f, 0.f),
		0,
		0,
		screenWidth,
		screenHeight,
		0,
		1,
		projMatrix,
		viewMatrix,
		XMMatrixIdentity());

	auto direction = dest - orig;
	direction = XMVector3Normalize(direction);
	//bool intersecting = entity->GetBoundingSphere().Intersects(orig, direction, distance);
	bool intersecting = boundingBox.Intersects(orig, direction, distance);
	return intersecting;
}