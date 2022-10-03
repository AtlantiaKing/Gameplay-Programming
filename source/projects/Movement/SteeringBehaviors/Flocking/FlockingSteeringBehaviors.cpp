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
	// Get the average position of all neighbors of the agent
	Vector2 averageNeighborPos{};

	if (m_pFlock->GetNrOfNeighbors() > 0)
	{
		averageNeighborPos = m_pFlock->GetAverageNeighborPos();

		// Set the target to the average position
		m_Target = averageNeighborPos;
	}
	else
	{
		averageNeighborPos = pAgent->GetPosition();
	}


	// Return the result of the seek steering
	return Seek::CalculateSteering(deltaT, pAgent);
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// Get all data about the neighbors
	const std::vector<SteeringAgent*> pNeightbors = m_pFlock->GetNeighbors();
	const int nrNeighbors{ m_pFlock->GetNrOfNeighbors() };
	const float neighborhoodRadius{ m_pFlock->GetNeighborhoodRadius() };

	// Calculate the seperation direction
	Vector2 cohesionDirection{};
	for (int i{}; i < nrNeighbors; ++i)
	{
		// Get the direction (and distance) away from the neighbor
		const Vector2 neighborDirection{ pAgent->GetPosition() - pNeightbors[i]->GetPosition() };
		const float neighBorDistance{ neighborDirection.Magnitude() };

		// Calculate the seperation vector depending on the distance of the neighbor (the furthur away the neighbor, the smaller the vector)
		cohesionDirection += neighborDirection * (1.0f - neighBorDistance / neighborhoodRadius);
	}

	// Set the target to the seperation target
	m_Target = pAgent->GetPosition() + cohesionDirection;

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
