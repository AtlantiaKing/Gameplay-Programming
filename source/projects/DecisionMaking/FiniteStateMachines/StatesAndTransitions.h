/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"

//------------
//---STATES---
//------------
namespace FSMStates
{
	class WanderState : public Elite::FSMState
	{
	public:
		WanderState() : Elite::FSMState{} {};

		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	private:
	};

	class SeekFoodState : public Elite::FSMState
	{
	public:
		SeekFoodState() : Elite::FSMState{} {};

		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	private:
	};

	class FleeFromTargetState : public Elite::FSMState
	{
	public:
		FleeFromTargetState() : Elite::FSMState{} {};

		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	private:
	};

	class SeekToTargetState : public Elite::FSMState
	{
	public:
		SeekToTargetState() : Elite::FSMState{} {};

		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	private:
	};

	class FleeFromBorderState : public Elite::FSMState
	{
	public:
		FleeFromBorderState() : Elite::FSMState{} {};

		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	private:
	};

	class FleeFromBorderAndTargetState : public Elite::FSMState
	{
	public:
		FleeFromBorderAndTargetState() : Elite::FSMState{} {};

		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	private:
	};
}

//-----------------
//---TRANSITIONS---
//-----------------
namespace FSMConditions
{
	class FoodNearByCondition : public Elite::FSMCondition
	{
	public:
		FoodNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class OtherFoodNearByCondition : public Elite::FSMCondition
	{
	public:
		OtherFoodNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class NoFoodNearByCondition : public Elite::FSMCondition
	{
	public:
		NoFoodNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class BiggerEnemyNearByCondition : public Elite::FSMCondition
	{
	public:
		BiggerEnemyNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class OtherBiggerEnemyNearByCondition : public Elite::FSMCondition
	{
	public:
		OtherBiggerEnemyNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class NoBiggerEnemyNearByCondition : public Elite::FSMCondition
	{
	public:
		NoBiggerEnemyNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class SmallerEnemyNearByCondition : public Elite::FSMCondition
	{
	public:
		SmallerEnemyNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};
	class OtherSmallerEnemyNearByCondition : public Elite::FSMCondition
	{
	public:
		OtherSmallerEnemyNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};
	class NoSmallerEnemyNearByCondition : public Elite::FSMCondition
	{
	public:
		NoSmallerEnemyNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class BorderNearByCondition : public Elite::FSMCondition
	{
	public:
		BorderNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class NoBorderNearByCondition : public Elite::FSMCondition
	{
	public:
		NoBorderNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class BorderAndBiggerEnemyNearByCondition : public Elite::FSMCondition
	{
	public:
		BorderAndBiggerEnemyNearByCondition() : Elite::FSMCondition{} {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};
}

#endif