#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;
class CellSpace;

class Flock final
{
public:
	Flock(
		int flockSize = 50, 
		float worldSize = 100.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI() ;
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }
	const std::vector<SteeringAgent*>& GetNeighbors() const { return m_pNeighbors; }

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;
	float GetNeighborhoodRadius() const;

	void SetTarget_Seek(TargetData target);
	void SetWorldTrimSize(float size) { m_WorldSize = size; }

private:
	//Datamembers
	int m_FlockSize = 0;
	std::vector<SteeringAgent*> m_pAgents;
	std::vector<SteeringAgent*> m_pNeighbors;

	bool m_IsDebugingAgent{};
	int m_AgentToDebug = 0;
	int m_AgentToChangeData = 0;

	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;

	float m_NeighborhoodRadius = 5.0f;
	int m_NrOfNeighbors = 0;

	const int m_NrPartitionsInAxis = 25;
	CellSpace* m_pCellSpace = nullptr;
	bool m_IsUsingPartitioning = true;
	bool m_DrawPartitioning = true;

	SteeringAgent* m_pAgentToEvade = nullptr;
	
	//Steering Behaviors
	Seek* m_pSeekBehavior = nullptr;
	Separation* m_pSeparationBehavior = nullptr;
	Cohesion* m_pCohesionBehavior = nullptr;
	VelocityMatch* m_pVelMatchBehavior = nullptr;
	Wander* m_pWanderBehavior = nullptr;
	Evade* m_pEvadeBehavior = nullptr;

	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;

	float* GetWeight(ISteeringBehavior* pBehaviour);

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};