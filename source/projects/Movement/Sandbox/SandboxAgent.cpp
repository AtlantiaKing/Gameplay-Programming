#include "stdafx.h"
#include "SandboxAgent.h"

using namespace Elite;

SandboxAgent::SandboxAgent(): BaseAgent()
{
	m_Target = GetPosition();
}

void SandboxAgent::Update(float dt)
{
	const float maxSpeed{ 50.0f };
	const float arrivalRadius{ 1.0f };
	const float slowRadius{ 15.0f };

	const Vector2 toTarget{ m_Target - GetPosition() };
	const float distance{ toTarget.Magnitude() };

	if (distance < arrivalRadius)
	{
		SetLinearVelocity({ 0.0f, 0.0f });
		return;
	}

	Vector2 velocity = toTarget.GetNormalized();

	if (distance <  slowRadius)
	{
		velocity *= maxSpeed * distance / slowRadius;
	}
	else
	{
		velocity *= maxSpeed;
	}

	SetLinearVelocity(velocity);

	//Orientation
	AutoOrient();
}

void SandboxAgent::Render(float dt)
{
	BaseAgent::Render(dt); //Default Agent Rendering
}

void SandboxAgent::AutoOrient()
{
	//Determine angle based on direction
	Vector2 velocity = GetLinearVelocity();
	if (velocity.Magnitude() > 0)
	{
		velocity.Normalize();
		SetRotation(atan2f(velocity.y, velocity.x) + static_cast<float>(E_PI_2));
	}

	SetRotation(GetRotation() + static_cast<float>(E_PI_2));
}