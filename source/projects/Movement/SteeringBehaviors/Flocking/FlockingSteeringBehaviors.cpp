#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

using namespace Elite;

//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// If there are agents, set the target to the average position of the neighbors, otherwise set the target to its own position
	if (m_pFlock->GetNrOfNeighbors() > 0)
	{
		m_Target = m_pFlock->GetAverageNeighborPos();
	}
	else
	{
		m_Target = pAgent->GetPosition();
	}

	// Return the result of the seek steering
	return Seek::CalculateSteering(deltaT, pAgent);
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// Get all data about the neighbors
	const std::vector<SteeringAgent*>& pNeightbors = m_pFlock->GetNeighbors();
	const int nrNeighbors{ m_pFlock->GetNrOfNeighbors() };
	const float neighborhoodRadius{ m_pFlock->GetNeighborhoodRadius() };

	// Calculate the seperation direction
	Vector2 seperationDirection{};

	// For each neighbor
	for (int i{}; i < nrNeighbors; ++i)
	{
		// Get the direction (and distance) away from the neighbor
		const Vector2 neighborDirection{ pAgent->GetPosition() - pNeightbors[i]->GetPosition() };
		const float neighborDistance{ neighborDirection.Magnitude() };

		// Calculate the seperation vector depending on the distance of the neighbor (the furthur away the neighbor, the smaller the vector)
		seperationDirection += neighborDirection * (1.0f - neighborDistance / neighborhoodRadius);
	}

	// Set the target to the seperation target
	m_Target = pAgent->GetPosition() + seperationDirection;

	// Return the result of the seek steering
	return Seek::CalculateSteering(deltaT, pAgent);
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	if (m_pFlock->GetNrOfNeighbors() > 0)
	{
		steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();
	}
	else
	{
		steering.LinearVelocity = pAgent->GetLinearVelocity();
	}

	return steering;
}
