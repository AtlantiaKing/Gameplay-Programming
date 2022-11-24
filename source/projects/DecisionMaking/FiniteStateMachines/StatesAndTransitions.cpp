#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

#define FOOD_SEARCH_RADIUS 50
#define ENEMY_SEARCH_RADIUS 20

void WanderState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	pAgent->SetToWander();
}

void SeekFoodState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	AgarioFood* pNearestFood;
	if (!pBlackboard->GetData("NearestFood", pNearestFood) || !pNearestFood) return;

	pAgent->SetToSeek(pNearestFood->GetPosition());
}

void FSMStates::FleeFromTargetState::Update(Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	AgarioAgent* pTarget;
	if (!pBlackboard->GetData("Target", pTarget) || !pTarget) return;

	pAgent->SetToFlee(pTarget->GetPosition());
}

void FSMStates::SeekToTargetState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	AgarioAgent* pTarget;
	if (!pBlackboard->GetData("Target", pTarget) || !pTarget) return;

	pAgent->SetToSeek(pTarget->GetPosition());
}

void FSMStates::FleeFromBorderState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	float worldSize;
	if (!pBlackboard->GetData("WorldSize", worldSize)) return;

	const float agentRadius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
	const Vector2 agentPosition{ pAgent->GetPosition() };

	Vector2 fleeTarget{ agentPosition };

	if (!(agentRadius <= agentPosition.x && agentPosition.x <= worldSize - agentRadius))
	{
		if (agentPosition.x > worldSize / 2.0f)
		{
			fleeTarget.x = worldSize;
		}
		else
		{
			fleeTarget.x = 0.0f;
		}
	}
	else
	{
		if (agentPosition.y > worldSize / 2.0f)
		{
			fleeTarget.y = worldSize;
		}
		else
		{
			fleeTarget.y = 0.0f;
		}
	}

	pAgent->SetToFlee(fleeTarget);
}


void FSMStates::FleeFromBorderAndTargetState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	float worldSize;
	if (!pBlackboard->GetData("WorldSize", worldSize)) return;

	const float agentRadius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
	const Vector2 agentPosition{ pAgent->GetPosition() };

	Vector2 borderTarget{ agentPosition };

	if (!(agentRadius <= agentPosition.x && agentPosition.x <= worldSize - agentRadius))
	{
		if (agentPosition.x > worldSize / 2.0f)
		{
			borderTarget.x = worldSize;
		}
		else
		{
			borderTarget.x = 0.0f;
		}
	}
	else
	{
		if (agentPosition.y > worldSize / 2.0f)
		{
			borderTarget.y = worldSize;
		}
		else
		{
			borderTarget.y = 0.0f;
		}
	}

	AgarioAgent* pAgentTarget;
	if (!pBlackboard->GetData("Target", pAgentTarget) || !pAgentTarget) return;

	const Vector2 pFleePosition{ (borderTarget + pAgentTarget->GetPosition()) / 2.0f };

	pAgent->SetToFlee(pFleePosition);
}

bool FoodNearByCondition::Evaluate(Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioFood*>* pFoodVector;
	if (!pBlackboard->GetData("FoodVector", pFoodVector) || !pFoodVector) return false;

	const float radius{ pAgent->GetRadius() + FOOD_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	auto isCloser
	{ 
		[agentPos](AgarioFood* pFood1, AgarioFood* pFood2) 
		{
			float dist1{ pFood1->GetPosition().DistanceSquared(agentPos) };
			float dist2{ pFood2->GetPosition().DistanceSquared(agentPos) };

			return dist1 < dist2;
		}
	};

	auto closestElementIt{ std::min_element(pFoodVector->begin(), pFoodVector->end(), isCloser) };

	if (closestElementIt == pFoodVector->end()) return false;

	AgarioFood* pClosestFood{ *closestElementIt };

	if (pClosestFood->GetPosition().DistanceSquared(agentPos) > Square(radius)) return false;

	pBlackboard->ChangeData("NearestFood", pClosestFood);
	return true;
}

bool FSMConditions::NoFoodNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioFood*>* pFoodVector;
	if (!pBlackboard->GetData("FoodVector", pFoodVector) || !pFoodVector) return false;

	const float radius{ pAgent->GetRadius() + FOOD_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	auto isCloser
	{
		[agentPos](AgarioFood* pFood1, AgarioFood* pFood2)
		{
			float dist1{ pFood1->GetPosition().DistanceSquared(agentPos) };
			float dist2{ pFood2->GetPosition().DistanceSquared(agentPos) };

			return dist1 < dist2;
		}
	};

	auto closestElementIt{ std::min_element(pFoodVector->begin(), pFoodVector->end(), isCloser) };

	if (closestElementIt == pFoodVector->end()) return true;

	AgarioFood* pClosestFood{ *closestElementIt };

	if (pClosestFood->GetPosition().DistanceSquared(agentPos) < Square(radius)) return false;

	return true;
}

bool FSMConditions::OtherFoodNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioFood*>* pFoodVector;
	if (!pBlackboard->GetData("FoodVector", pFoodVector) || !pFoodVector) return false;

	const float radius{ pAgent->GetRadius() + FOOD_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	auto isCloser
	{
		[agentPos](AgarioFood* pFood1, AgarioFood* pFood2)
		{
			float dist1{ pFood1->GetPosition().DistanceSquared(agentPos) };
			float dist2{ pFood2->GetPosition().DistanceSquared(agentPos) };

			return dist1 < dist2;
		}
	};

	auto closestElementIt{ std::min_element(pFoodVector->begin(), pFoodVector->end(), isCloser) };

	if (closestElementIt == pFoodVector->end()) return false;

	AgarioFood* pCurClosestFood{};
	pBlackboard->GetData("NearestFood", pCurClosestFood);

	AgarioFood* pClosestFood{ *closestElementIt };

	if (pClosestFood->GetPosition().DistanceSquared(agentPos) > Square(radius)) return false;

	if (pClosestFood == pCurClosestFood) return false;

	pBlackboard->ChangeData("NearestFood", pClosestFood);
	return true;
}

bool FSMConditions::BiggerEnemyNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioAgent*>* pAgentVector;
	if (!pBlackboard->GetData("AgentVector", pAgentVector) || !pAgentVector) return false;

	const float radius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	AgarioAgent* closestBiggerEnemy{};
	float closestEnemyDistance{ FLT_MAX };

	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		const float distance{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

		if (distance > Square(radius + pOtherAgent->GetRadius())) continue;

		if (distance > closestEnemyDistance) continue;

		if (pOtherAgent->GetRadius() < pAgent->GetRadius()) continue;

		if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 1.0f) continue;

		closestBiggerEnemy = pOtherAgent;
		closestEnemyDistance = distance;
	}

	if (!closestBiggerEnemy) return false;

	pBlackboard->ChangeData("Target", closestBiggerEnemy);
	return true;
}

bool FSMConditions::OtherBiggerEnemyNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioAgent*>* pAgentVector;
	if (!pBlackboard->GetData("AgentVector", pAgentVector) || !pAgentVector) return false;

	const float radius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	AgarioAgent* closestBiggerEnemy{};
	float closestEnemyDistance{ FLT_MAX };

	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		const float distance{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

		if (distance > Square(radius + pOtherAgent->GetRadius())) continue;

		if (distance > closestEnemyDistance) continue;

		if (pOtherAgent->GetRadius() < pAgent->GetRadius()) continue;

		if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 1.0f) continue;

		closestBiggerEnemy = pOtherAgent;
		closestEnemyDistance = distance;
	}

	if (!closestBiggerEnemy) return false;

	AgarioAgent* curTarget;
	pBlackboard->GetData("Target", curTarget);

	if (closestBiggerEnemy == curTarget) return false;

	pBlackboard->ChangeData("Target", closestBiggerEnemy);
	return true;
}

bool FSMConditions::NoBiggerEnemyNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioAgent*>* pAgentVector;
	if (!pBlackboard->GetData("AgentVector", pAgentVector) || !pAgentVector) return false;

	const float radius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	AgarioAgent* closestBiggerEnemy{};
	float closestEnemyDistance{ FLT_MAX };

	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		const float distance{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

		if (distance > Square(radius + pOtherAgent->GetRadius())) continue;

		if (distance > closestEnemyDistance) continue;

		if (pOtherAgent->GetRadius() < pAgent->GetRadius()) continue;

		if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 1.0f) continue;

		closestBiggerEnemy = pOtherAgent;
		closestEnemyDistance = distance;
	}

	if (closestBiggerEnemy) return false;

	pBlackboard->ChangeData("Target", static_cast<AgarioAgent*>(nullptr));
	return true;
}

bool FSMConditions::SmallerEnemyNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioAgent*>* pAgentVector;
	if (!pBlackboard->GetData("AgentVector", pAgentVector) || !pAgentVector) return false;

	const float radius{ pAgent->GetRadius() + FOOD_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	AgarioAgent* closestSmallerEnemy{};
	float closestEnemyDistance{ FLT_MAX };

	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		const float distance{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

		if (distance > Square(radius + pOtherAgent->GetRadius())) continue;

		if (distance > closestEnemyDistance) continue;

		if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 2.0f) continue;

		if (pOtherAgent->GetRadius() > pAgent->GetRadius()) continue;

		closestSmallerEnemy = pOtherAgent;
		closestEnemyDistance = distance;
	}

	if (!closestSmallerEnemy) return false;

	pBlackboard->ChangeData("Target", closestSmallerEnemy);
	return true;
}

bool FSMConditions::OtherSmallerEnemyNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioAgent*>* pAgentVector;
	if (!pBlackboard->GetData("AgentVector", pAgentVector) || !pAgentVector) return false;

	const float radius{ pAgent->GetRadius() + FOOD_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	AgarioAgent* closestSmallerEnemy{};
	float closestEnemyDistance{ FLT_MAX };

	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		const float distance{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

		if (distance > Square(radius + pOtherAgent->GetRadius())) continue;

		if (distance > closestEnemyDistance) continue;

		if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 2.0f) continue;

		if (pOtherAgent->GetRadius() > pAgent->GetRadius()) continue;

		closestSmallerEnemy = pOtherAgent;
		closestEnemyDistance = distance;
	}

	if (!closestSmallerEnemy) return false;

	AgarioAgent* curTarget;
	pBlackboard->GetData("Target", curTarget);

	if (closestSmallerEnemy == curTarget) return false;

	pBlackboard->ChangeData("Target", closestSmallerEnemy);
	return true;
}

bool FSMConditions::NoSmallerEnemyNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioAgent*>* pAgentVector;
	if (!pBlackboard->GetData("AgentVector", pAgentVector) || !pAgentVector) return false;

	const float radius{ pAgent->GetRadius() + FOOD_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	AgarioAgent* closestSmallerEnemy{};
	float closestEnemyDistance{ FLT_MAX };

	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		const float distance{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

		if (distance > Square(radius + pOtherAgent->GetRadius())) continue;

		if (distance > closestEnemyDistance) continue;

		if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 2.0f) continue;

		if (pOtherAgent->GetRadius() > pAgent->GetRadius()) continue;

		closestSmallerEnemy = pOtherAgent;
		closestEnemyDistance = distance;
	}

	if (closestSmallerEnemy) return false;

	pBlackboard->ChangeData("Target", static_cast<AgarioAgent*>(nullptr));
	return true;
}

bool FSMConditions::BorderNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	float worldSize;
	if (!pBlackboard->GetData("WorldSize", worldSize)) return false;

	const float agentRadius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
	const Vector2 agentPosition{ pAgent->GetPosition() };

	return !(agentRadius <= agentPosition.x && agentPosition.x <= worldSize - agentRadius &&
		agentRadius <= agentPosition.y && agentPosition.y <= worldSize - agentRadius);
}

bool FSMConditions::BorderAndBiggerEnemyNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	std::vector<AgarioAgent*>* pAgentVector;
	if (!pBlackboard->GetData("AgentVector", pAgentVector) || !pAgentVector) return false;

	const float radius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	AgarioAgent* closestBiggerEnemy{};
	float closestEnemyDistance{ FLT_MAX };

	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		const float distance{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

		if (distance > Square(radius + pOtherAgent->GetRadius())) continue;

		if (distance > closestEnemyDistance) continue;

		if (pOtherAgent->GetRadius() < pAgent->GetRadius()) continue;

		closestBiggerEnemy = pOtherAgent;
		closestEnemyDistance = distance;
	}

	float worldSize;
	if (!pBlackboard->GetData("WorldSize", worldSize)) return false;

	const Vector2 agentPosition{ pAgent->GetPosition() };

	return (!(radius <= agentPosition.x && agentPosition.x <= worldSize - radius &&
		radius <= agentPosition.y && agentPosition.y <= worldSize - radius) && closestBiggerEnemy);
}

bool FSMConditions::NoBorderNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;

	float worldSize;
	if (!pBlackboard->GetData("WorldSize", worldSize)) return false;

	const float agentRadius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
	const Vector2 agentPosition{ pAgent->GetPosition() };

	return agentRadius <= agentPosition.x && agentPosition.x <= worldSize - agentRadius &&
		agentRadius <= agentPosition.y && agentPosition.y <= worldSize - agentRadius;
}
