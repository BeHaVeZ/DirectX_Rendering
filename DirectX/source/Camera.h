#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

class Matrix;


struct Camera
{
	Camera() = default;

	Camera(const Vector3& _origin, float _fovAngle) :
		origin{ _origin },
		fovAngle{ _fovAngle }
	{
	}


	Vector3 origin{};
	float fovAngle{ 90.f };
	float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
	float moveSpeed{ 30 };
	float lookSensitivity{ 5.f };

	Vector3 forward{ Vector3::UnitZ };
	Vector3 up{ Vector3::UnitY };
	Vector3 right{ Vector3::UnitX };

	float totalPitch{};
	float totalYaw{};

	Matrix invViewMatrix{};
	Matrix viewMatrix{};
	Matrix projectionMatrix{};

	float nearPlane{ 0.1f };
	float farPlane{ 300.f };
	float aspectRatio{ 1.f };

	bool inspectMode{ false };

	void Initialize(float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f }, float _aspectRatio = 0)
	{
		assert(_fovAngle > 0.f && _fovAngle < 180.f);
		fovAngle = _fovAngle;
		assert(_aspectRatio != 0.f);
		aspectRatio = _aspectRatio;

		origin = _origin;
		fov = tanf((fovAngle * TO_RADIANS) / 2.f);
	}

	void CalculateViewMatrix()
	{
		//ONB => invViewMatrix
		//Inverse(ONB) => ViewMatrix
		if (inspectMode == false)
		{
			Matrix translationMatrix = Matrix::CreateTranslation(origin);
			Matrix rotationMatrix = Matrix::CreateRotation(totalPitch, totalYaw, 0.f);

			forward = rotationMatrix.TransformVector(Vector3::UnitZ);
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			invViewMatrix = rotationMatrix * translationMatrix;
			viewMatrix = invViewMatrix.Inverse();
		}
		else
		{
			////camera-orbit effect like cs skins in inventory
			//Matrix translationMatrix = Matrix::CreateTranslation(-origin);
			//Matrix rotationMatrix = Matrix::CreateRotation(totalPitch, totalYaw, 0.f);

			//viewMatrix = rotationMatrix * translationMatrix;

			//forward = viewMatrix.TransformVector(Vector3::UnitZ);
			//right = viewMatrix.TransformVector(Vector3::UnitX);
			//up = viewMatrix.TransformVector(Vector3::UnitY);
		}
	}

	void CalculateProjectionMatrix()
	{
		projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
		//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
	}

	void Update(const Timer* pTimer)
	{
		//Camera Update Logic
		//...

		// Keyboard Input
		const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

		// Mouse Input
		int mouseX{}, mouseY{};
		const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

		const float deltaTime = pTimer->GetElapsed();
		const float moveSpeedPerFrame = moveSpeed * deltaTime;
		const float lookSensPerFrame = lookSensitivity * deltaTime;

		// movement
		if (inspectMode == false)
		{
			MoveWithKeyboard(pKeyboardState, moveSpeedPerFrame);
		}

		//rotation
		RotateWithMouse(mouseState, mouseX, mouseY, moveSpeedPerFrame, lookSensPerFrame);



		//Update Matrices
		CalculateViewMatrix();
		CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
	}
	void MoveWithKeyboard(const uint8_t* pKeyboardState, float moveSpeedPerSecond)
	{
		const float moveRight = (pKeyboardState[SDL_SCANCODE_D] or pKeyboardState[SDL_SCANCODE_RIGHT]) ? 1.0f : 0.0f;
		const float moveLeft = (pKeyboardState[SDL_SCANCODE_Q] or pKeyboardState[SDL_SCANCODE_A] or pKeyboardState[SDL_SCANCODE_LEFT]) ? 1.0f : 0.0f;
		const float moveForward = (pKeyboardState[SDL_SCANCODE_W] or pKeyboardState[SDL_SCANCODE_Z] or pKeyboardState[SDL_SCANCODE_UP]) ? 1.0f : 0.0f;
		const float moveBackward = (pKeyboardState[SDL_SCANCODE_S] or pKeyboardState[SDL_SCANCODE_DOWN]) ? 1.0f : 0.0f;

		origin += moveRight * moveSpeedPerSecond * right;
		origin -= moveLeft * moveSpeedPerSecond * right;
		origin += moveForward * moveSpeedPerSecond * forward;
		origin -= moveBackward * moveSpeedPerSecond * forward;
	}
	void RotateWithMouse(uint32_t mouseState, int mouseX, int mouseY, float moveSpeedPerSecond, float lookSpeedPerSecond)
	{
		if (mouseState & SDL_BUTTON_LMASK and inspectMode == false)
		{
			if (mouseState & SDL_BUTTON_RMASK)
				origin.y += mouseY * moveSpeedPerSecond;
			else
			{
				origin.z += mouseY * moveSpeedPerSecond;
				totalYaw += mouseX * lookSpeedPerSecond;
			}
		}
		else if (mouseState & SDL_BUTTON_LMASK)
		{
			totalPitch += -mouseY * lookSpeedPerSecond;
			totalYaw += mouseX * lookSpeedPerSecond;
		}

		if (mouseState & SDL_BUTTON_RMASK && !(mouseState & SDL_BUTTON_LMASK))
		{
			totalPitch += -mouseY * lookSpeedPerSecond;
			totalYaw += mouseX * lookSpeedPerSecond;
		}
	}

	void SetInspectMode()
	{
		origin = { 0.f,0.f,-132.827f };
		totalPitch = 0.f;
		totalYaw = 0.f;
		inspectMode = !inspectMode;
	}

	const Matrix& GetViewMatrix() const { return viewMatrix; }
	const Matrix& GetInvMatrix() const { return invViewMatrix; }
	const Matrix& GetProjectionMatrix() const { return projectionMatrix; }

	const Matrix GetWorldViewProjection() const { return GetViewMatrix() * GetProjectionMatrix(); }
};
