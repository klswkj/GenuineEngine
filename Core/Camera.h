#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
	XMFLOAT4X4 inverseProjectionView;
	XMFLOAT3 position;
	XMFLOAT3 direction;
	XMFLOAT4 rotation;
	float rotationX;
	float rotationY;
	float nearZ;
	float farZ;
public:
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetDirection();
	void Update(float deltaTime);
	void SetPosition(const XMFLOAT3& pos);

	const XMFLOAT4X4&	GetViewMatrix();
	const XMFLOAT4X4&	GetProjectionMatrix();
	const XMFLOAT4X4&	GetInverseProjectionViewMatrix();
	const float&		GetNearZ();
	const float&		GetFarZ();

	XMFLOAT4X4 GetViewProjectionMatrixTransposed();
	XMFLOAT4X4 GetViewProjectionMatrix();
	XMFLOAT4X4 GetViewMatrixTransposed();
	XMFLOAT4X4 GetProjectionMatrixTransposed();

	void Rotate(float x, float y);
	void SetProjectionMatrix(float width, float height);
	Camera(float width, float height, float nearZ = 0.1f, float farZ = 1000.f);
	~Camera();
};