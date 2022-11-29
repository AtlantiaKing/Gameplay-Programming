/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) 
			return Elite::BehaviorState::Failure;

		pAgent->SetToWander();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;

		Elite::Vector2 targetPos;
		if (!pBlackboard->GetData("Target", targetPos))
			return Elite::BehaviorState::Failure;

		pAgent->SetToSeek(targetPos);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;

		AgarioAgent* targetAgent;
		if (!pBlackboard->GetData("AgentFleeTarget", targetAgent) || !targetAgent)
			return Elite::BehaviorState::Failure;

		pAgent->SetToFlee(targetAgent->GetPosition());

		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return false;

		std::vector<AgarioFood*>* pFoodVec;
		if (!pBlackboard->GetData("FoodVec", pFoodVec))
			return false;

		const float searchRadius{ pAgent->GetRadius() + 40.0f };

		AgarioFood* pClosestFood{};
		float closestDistanceSqr{ searchRadius * searchRadius };
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, { 0.0f, 1.0f, 0.0f }, DEBUGRENDERER2D->NextDepthSlice());

		for (AgarioFood* pFood : *pFoodVec)
		{
			const float distanceSqr{ pFood->GetPosition().DistanceSquared(agentPos) };

			if (distanceSqr < closestDistanceSqr)
			{
				closestDistanceSqr = distanceSqr;
				pClosestFood = pFood;
			}
		}

		if (!pClosestFood) return false;

		pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
		return true;
	}

	bool IsBiggerEnemyNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

		std::vector<AgarioAgent*>* pAgentVector;
		if (!pBlackboard->GetData("AgentsVec", pAgentVector) || !pAgentVector) return false;

		const float radius{ pAgent->GetRadius() + 20.0f };
		const Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, radius, { 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

		AgarioAgent* closestBiggerEnemy{};
		float closestEnemyDistance{ FLT_MAX };

		for (AgarioAgent* pOtherAgent : *pAgentVector)
		{
			if (!pOtherAgent || pOtherAgent == pAgent) continue;

			const float distance{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

			if (distance > Elite::Square(radius + pOtherAgent->GetRadius())) continue;

			if (distance > closestEnemyDistance) continue;

			if (pOtherAgent->GetRadius() < pAgent->GetRadius()) continue;

			if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 1.0f) continue;

			closestBiggerEnemy = pOtherAgent;
			closestEnemyDistance = distance;
		}

		if (!closestBiggerEnemy) return false;

		pBlackboard->ChangeData("AgentFleeTarget", closestBiggerEnemy);
		return true;
	}

	bool IsSmallerEnemyNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

		std::vector<AgarioAgent*>* pAgentVector;
		if (!pBlackboard->GetData("AgentsVec", pAgentVector) || !pAgentVector) return false;

		const float radius{ pAgent->GetRadius() + 30.0f };
		const Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, radius, { 0.0f, 1.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

		AgarioAgent* closestSmallerEnemy{};
		float closestEnemyDistance{ FLT_MAX };

		for (AgarioAgent* pOtherAgent : *pAgentVector)
		{
			if (!pOtherAgent || pOtherAgent == pAgent) continue;

			const float distance{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

			if (distance > Elite::Square(radius + pOtherAgent->GetRadius())) continue;

			if (distance > closestEnemyDistance) continue;

			if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 2.0f) continue;

			if (pOtherAgent->GetRadius() > pAgent->GetRadius()) continue;

			closestSmallerEnemy = pOtherAgent;
			closestEnemyDistance = distance;
		}

		if (!closestSmallerEnemy) return false;

		pBlackboard->ChangeData("Target", closestSmallerEnemy->GetPosition());
		return true;
	}
}

#endif