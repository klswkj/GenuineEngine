#include "stdafx.h"
#include "Input.h"

void InputManager::Update()
{
	InputState newState;
	newState.KeyboardState = context.KeyboardInstance->GetState();
	newState.MouseState = context.MouseInstance->GetState();
	state = newState;
}

const DirectX::Keyboard::State InputManager::GetKeyboardState()
{
	return ((InputState)this->state).KeyboardState;
}

const DirectX::Mouse::State InputManager::GetMouseState()
{
	return ((InputState)this->state).MouseState;
}

bool InputManager::IsKeyDown(DirectX::Keyboard::Keys key)
{
	auto kb = GetKeyboardState();
	return kb.IsKeyDown(key);
}

bool InputManager::IsKeyUp(DirectX::Keyboard::Keys key)
{
	auto kb = GetKeyboardState();
	return kb.IsKeyUp(key);
}

bool InputManager::IsMouseDown(MouseButton button)
{
	auto mouse = GetMouseState();
	switch (button)
	{
	case MBLeft:
		return mouse.leftButton;
	case MBRight:
		return mouse.rightButton;
	case MBMiddle:
		return mouse.middleButton;
	}

	return false;
}

bool Input::IsKeyDown(DirectX::Keyboard::Keys key)
{
	auto kb = InputManager::GetInstance().GetKeyboardState();
	return kb.IsKeyDown(key);
}

bool Input::IsKeyUp(DirectX::Keyboard::Keys key)
{
	auto kb = InputManager::GetInstance().GetKeyboardState();
	return kb.IsKeyUp(key);
}

bool Input::IsMouseDown(MouseButton button)
{
	auto mouse = InputManager::GetInstance().GetMouseState();
	switch (button)
	{
	case MBLeft:
		return mouse.leftButton;
	case MBRight:
		return mouse.rightButton;
	case MBMiddle:
		return mouse.middleButton;
	}

	return false;
}
