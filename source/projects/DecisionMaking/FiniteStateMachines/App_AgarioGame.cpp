#include "stdafx.h"
#include "App_AgarioGame.h"
#include "StatesAndTransitions.h"


//AgarioIncludes
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioContactListener.h"


using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

App_AgarioGame::App_AgarioGame()
{
}

App_AgarioGame::~App_AgarioGame()
{
	for (auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for (auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	SAFE_DELETE(m_pContactListener);
	SAFE_DELETE(m_pCustomAgent);
	for (auto& s : m_pStates)
	{
		SAFE_DELETE(s);
	}

	for (auto& t : m_pConditions)
	{
		SAFE_DELETE(t);
	}

}

void App_AgarioGame::Start()
{
	// New seed for the random positions
	srand(time(NULL));

	// Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	// Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	// Create common states
	WanderState* pWanderState{ new WanderState{} };
	m_pStates.push_back(pWanderState);

	// Create default agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
		AgarioAgent* newAgent = new AgarioAgent(randomPos);

		Blackboard* pBlackBoard{ CreateBlackboard(newAgent) };

		FiniteStateMachine* pStateMachine{ new FiniteStateMachine{ pWanderState, pBlackBoard } };

		newAgent->SetDecisionMaking(pStateMachine);

		m_pAgentVec.push_back(newAgent);
	}

	//-------------------
	//Create Custom Agent
	//-------------------
	Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
	Color customColor = Color{ 0.0f, 1.0f, 0.0f };
	m_pCustomAgent = new AgarioAgent(randomPos, customColor);

	//1. Create and add the necessary blackboard data
	Blackboard* pBlackBoard{ CreateBlackboard(m_pCustomAgent) };

	//2. Create the different agent states
	SeekFoodState* pSeekFoodState{ new SeekFoodState{} };
	m_pStates.push_back(pSeekFoodState);
	FleeFromTargetState* pFleeFromTargetState{ new FleeFromTargetState{} };
	m_pStates.push_back(pFleeFromTargetState);
	SeekToTargetState* pSeekToTargetState{ new SeekToTargetState{} };
	m_pStates.push_back(pSeekToTargetState);
	FleeFromBorderState* pFleeFromBorderState{ new FleeFromBorderState{} };
	m_pStates.push_back(pFleeFromBorderState);
	FleeFromBorderAndTargetState* pFleeFromBorderAndTargetState{ new FleeFromBorderAndTargetState{} };
	m_pStates.push_back(pFleeFromBorderAndTargetState);

	//3. Create the conditions to transition between those states
	FoodNearByCondition* pFoodNearBy{ new FoodNearByCondition{} };
	m_pConditions.push_back(pFoodNearBy);
	OtherFoodNearByCondition* pOtherFoodNearBy{ new OtherFoodNearByCondition{} };
	m_pConditions.push_back(pOtherFoodNearBy);
	NoFoodNearByCondition* pNoFoodNearBy{ new NoFoodNearByCondition{} };
	m_pConditions.push_back(pNoFoodNearBy);

	BiggerEnemyNearByCondition* pBiggerEnemyNearBy{ new BiggerEnemyNearByCondition{} };
	m_pConditions.push_back(pBiggerEnemyNearBy);
	OtherBiggerEnemyNearByCondition* pOtherBiggerEnemyNearBy{ new OtherBiggerEnemyNearByCondition{} };
	m_pConditions.push_back(pOtherBiggerEnemyNearBy);
	NoBiggerEnemyNearByCondition* pNoBiggerEnemyNearBy{ new NoBiggerEnemyNearByCondition{} };
	m_pConditions.push_back(pNoBiggerEnemyNearBy);

	SmallerEnemyNearByCondition* pSmallerEnemyNearBy{ new SmallerEnemyNearByCondition{} };
	m_pConditions.push_back(pSmallerEnemyNearBy);
	OtherSmallerEnemyNearByCondition* pOtherSmallerEnemyNearBy{ new OtherSmallerEnemyNearByCondition{} };
	m_pConditions.push_back(pOtherSmallerEnemyNearBy);
	NoSmallerEnemyNearByCondition* pNoSmallerEnemyNearBy{ new NoSmallerEnemyNearByCondition{} };
	m_pConditions.push_back(pNoSmallerEnemyNearBy);

	BorderNearByCondition* pBorderNearBy{ new BorderNearByCondition{} };
	m_pConditions.push_back(pBorderNearBy);
	BorderAndBiggerEnemyNearByCondition* pBorderAndBiggerEnemyNearBy{ new BorderAndBiggerEnemyNearByCondition{} };
	m_pConditions.push_back(pBorderAndBiggerEnemyNearBy);
	NoBorderNearByCondition* pNoBorderNearBy{ new NoBorderNearByCondition{} };
	m_pConditions.push_back(pNoBorderNearBy);

	//4. Create the finite state machine with a starting state and the blackboard
	FiniteStateMachine* pStateMachine{ new FiniteStateMachine{ pWanderState, pBlackBoard } };
	
	//5. Add the transitions for the states to the state machine
	// stateMachine->AddTransition(startState, toState, condition)
	// startState: active state for which the transition will be checked
	// condition: if the Evaluate function returns true => transition will fire and move to the toState
	// toState: end state where the agent will move to if the transition fires
	pStateMachine->AddTransition(pWanderState, pFleeFromTargetState, pBiggerEnemyNearBy);
	pStateMachine->AddTransition(pWanderState, pSeekToTargetState, pSmallerEnemyNearBy);
	pStateMachine->AddTransition(pWanderState, pSeekFoodState, pFoodNearBy);
	pStateMachine->AddTransition(pWanderState, pFleeFromBorderState, pBorderNearBy);

	pStateMachine->AddTransition(pSeekFoodState, pFleeFromTargetState, pBiggerEnemyNearBy);
	pStateMachine->AddTransition(pSeekFoodState, pSeekToTargetState, pSmallerEnemyNearBy);
	pStateMachine->AddTransition(pSeekFoodState, pSeekFoodState, pOtherFoodNearBy);
	pStateMachine->AddTransition(pSeekFoodState, pWanderState, pNoFoodNearBy);

	pStateMachine->AddTransition(pFleeFromTargetState, pFleeFromBorderAndTargetState, pBorderAndBiggerEnemyNearBy);
	pStateMachine->AddTransition(pFleeFromTargetState, pFleeFromTargetState, pOtherBiggerEnemyNearBy);
	pStateMachine->AddTransition(pFleeFromTargetState, pWanderState, pNoBiggerEnemyNearBy);

	pStateMachine->AddTransition(pSeekToTargetState, pFleeFromTargetState, pBiggerEnemyNearBy);
	pStateMachine->AddTransition(pSeekToTargetState, pSeekToTargetState, pOtherSmallerEnemyNearBy);
	pStateMachine->AddTransition(pSeekToTargetState, pWanderState, pNoSmallerEnemyNearBy);

	pStateMachine->AddTransition(pFleeFromBorderState, pFleeFromBorderAndTargetState, pBorderAndBiggerEnemyNearBy);
	pStateMachine->AddTransition(pFleeFromBorderState, pSeekToTargetState, pSmallerEnemyNearBy);
	pStateMachine->AddTransition(pFleeFromBorderState, pSeekFoodState, pFoodNearBy);
	pStateMachine->AddTransition(pFleeFromBorderState, pWanderState, pNoBorderNearBy);

	pStateMachine->AddTransition(pFleeFromBorderAndTargetState, pFleeFromTargetState, pOtherBiggerEnemyNearBy);
	pStateMachine->AddTransition(pFleeFromBorderAndTargetState, pFleeFromTargetState, pNoBorderNearBy);
	pStateMachine->AddTransition(pFleeFromBorderAndTargetState, pFleeFromBorderState, pNoBiggerEnemyNearBy);

	//6. Activate the decision making stucture on the custom agent by calling the SetDecisionMaking function
	m_pCustomAgent->SetDecisionMaking(pStateMachine);
	m_pCustomAgent->SetRenderBehavior(true);
}

void App_AgarioGame::Update(float deltaTime)
{
	UpdateImGui();

	//Check if agent is still alive
	if (m_pCustomAgent->CanBeDestroyed())
	{
		m_GameOver = true;

		//Update the other agents and food
		UpdateAgarioEntities(m_pFoodVec, deltaTime);
		UpdateAgarioEntities(m_pAgentVec, deltaTime);
		return;
	}
	//Update the custom agent
	m_pCustomAgent->Update(deltaTime);
	m_pCustomAgent->TrimToWorld(m_TrimWorldSize, false);

	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);

	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(0, m_TrimWorldSize)));
	}
}

void App_AgarioGame::Render(float deltaTime) const
{
	RenderWorldBounds(m_TrimWorldSize);

	for (AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for (AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pCustomAgent->Render(deltaTime);
}

Blackboard* App_AgarioGame::CreateBlackboard(AgarioAgent* a)
{
	Blackboard* pBlackboard = new Blackboard();
	pBlackboard->AddData("Agent", a);
	pBlackboard->AddData("FoodVector", &m_pFoodVec);
	pBlackboard->AddData("NearestFood", static_cast<AgarioFood*>(nullptr));
	pBlackboard->AddData("AgentVector", &m_pAgentVec);
	pBlackboard->AddData("Target", static_cast<AgarioAgent*>(nullptr));
	pBlackboard->AddData("WorldSize", m_TrimWorldSize);
	//...

	return pBlackboard;
}

void App_AgarioGame::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
		
		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f",m_pCustomAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if(m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width/2.0f- menuWidth, height/2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pCustomAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}
