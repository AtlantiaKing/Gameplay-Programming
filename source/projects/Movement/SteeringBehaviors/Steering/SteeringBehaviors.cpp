//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"
using namespace Elite;

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), {0.0f, 1.0f, 0.0f});
	}

	return steering;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.0f, { 0.0f, 1.0f, 0.0f });
	}

	return steering;
}

//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	const float slowRadius{ 15.0f };
	const float stopRadius{ 5.0f };
	const float velocityRunOffFactor{ pAgent->GetMaxLinearSpeed() / Square(slowRadius - stopRadius)};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	const float distance{ steering.LinearVelocity.Normalize() };

	if (distance > stopRadius)
	{
		if (distance < slowRadius)
		{
			steering.LinearVelocity *= Square(distance - stopRadius) * velocityRunOffFactor;
		}
		else
		{
			steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
		}
	}
	else
	{
		steering.LinearVelocity = {};
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, slowRadius, { 0.0f, 0.0f, 1.0f }, 0.0f);
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, stopRadius, { 1.0f, 0.0f, 0.0f }, 0.0f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), {0.0f, 1.0f, 0.0f});
	}

	return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	const float stopCos{ 0.0005f };

	const float curRotation{ pAgent->GetRotation()};

	const Vector2 curDirection{ cosf(curRotation), sinf(curRotation) };
	const Vector2 lookDirection{ m_Target.Position - pAgent->GetPosition() };

	const float dot{ curDirection.Dot(lookDirection.GetNormalized()) };

	if (dot < 1.0f - stopCos)
	{
		const float crossed{ curDirection.Cross(lookDirection) };

		steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
		if (crossed < 0)
		{
			steering.AngularVelocity *= -1.0f;
		}
		const float velocityOffset{ 0.05f };
		steering.AngularVelocity *= 1.0f - dot / (1.0f - stopCos) + 0.05f;
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), curDirection, 5.0f, { 0.0f, 1.0f, 0.0f });
	}

	return steering;
}

//WANDER
//****
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const Vector2 circleCenter{ pAgent->GetPosition() + pAgent->GetDirection() * m_OffsetDistance };

	const float randAngle{ float(rand()) / RAND_MAX * m_MaxAngleChange * 2 - m_MaxAngleChange };

	m_WanderAngle += randAngle;

	const Vector2 newPoint{ circleCenter + Vector2{ cosf(m_WanderAngle), sinf(m_WanderAngle) } * m_Radius };

	m_Target = newPoint;

	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(newPoint, 1.0f, { 1.0f, 0.0f, 0.0F }, 0.0f);
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, { 0.0f, 0.0f, 1.0f }, 0.0f);
	}

	return steering;
}

//PURSUIT
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Vector2 targetVel{ m_Target.LinearVelocity };
	const float targetVelMagnitude{ targetVel.Normalize() };

	const float followDistance{ 10.0f }; 
	const float curDistanceSqr{ pAgent->GetPosition().DistanceSquared(m_Target.Position) };

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, followDistance, { 1.0f, 0.0f, 0.0f }, 0.0f);
	}

	Vector2 targetOffset{ targetVel * (targetVelMagnitude + pAgent->GetLinearVelocity().Magnitude()) };
	if (curDistanceSqr < Square(followDistance))
	{
		targetOffset *= sqrtf(curDistanceSqr) / followDistance;
	}
	m_Target = m_Target.Position + targetOffset;

	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, 1.0f, { 0.0f, 0.0f, 1.0f }, 0.0f);
	}
;
	return steering;
}

//EVADE
//****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Vector2 targetVel{ m_Target.LinearVelocity };
	const float targetVelMagnitude{ targetVel.Normalize() };

	const float curDistanceSqr{ pAgent->GetPosition().DistanceSquared(m_Target.Position) };

	Vector2 targetOffset{ targetVel * (targetVelMagnitude + pAgent->GetLinearVelocity().Magnitude()) };
	m_Target = m_Target.Position + targetOffset;

	SteeringOutput steering{ Flee::CalculateSteering(deltaT, pAgent) };

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, 1.0f, { 0.0f, 0.0f, 1.0f }, 0.0f);
	}
	;
	return steering;
}
