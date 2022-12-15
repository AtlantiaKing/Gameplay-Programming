#include "stdafx.h"
#include "App_AgarioGame_IM.h"

//AgarioIncludes
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioContactListener.h"


using namespace Elite;

App_AgarioGame_IM::App_AgarioGame_IM()
{
}

App_AgarioGame_IM::~App_AgarioGame_IM()
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

	SAFE_DELETE(m_pInfluenceGrid);
}

void App_AgarioGame_IM::Start()
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

	// Create default agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
		AgarioAgent* newAgent = new AgarioAgent(randomPos);

		newAgent->SetToWander();

		m_pAgentVec.push_back(newAgent);
	}

	//-------------------
	//Create Custom Agent
	//-------------------
	Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
	Color customColor = Color{ 0.0f, 1.0f, 0.0f };
	m_pCustomAgent = new AgarioAgent(randomPos, customColor);

	//6. Activate the decision making stucture on the custom agent by calling the SetDecisionMaking function
	m_pCustomAgent->SetRenderBehavior(true);

	// Create the Influence Map
	m_pInfluenceGrid = new InfluenceMap<InfluenceGrid>(false);
	m_pInfluenceGrid->InitializeGrid(m_GridSize, m_GridSize, static_cast<int>(m_TrimWorldSize) / m_GridSize, false, true);
	m_pInfluenceGrid->InitializeBuffer();

	// Set the data of the Influence Map
	m_pInfluenceGrid->SetMomentum(0.25f);
	m_pInfluenceGrid->SetDecay(0.4f);
	m_pInfluenceGrid->SetPropagationInterval(0.1f);
}

void App_AgarioGame_IM::Update(float deltaTime)
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

	// Update the enemy positions on the influence grid
	const float gridTileSize{ m_TrimWorldSize / m_GridSize };
	const float minRadiusDelta{ 1 };
	for (AgarioAgent* pAgent : m_pAgentVec)
	{
		if (pAgent == m_pCustomAgent) continue;

		const Vector2 curPos{ pAgent->GetPosition() };
		const float curRadius{ pAgent->GetRadius() };

		const float radiusDifference{ m_pCustomAgent->GetRadius() - curRadius };
		if (abs(radiusDifference) <= minRadiusDelta) continue;
		const float enemyInfluence{ radiusDifference > minRadiusDelta ? 100.0f : -100.0f };

		for (int x{ static_cast<int>((curPos.x - curRadius) / gridTileSize) }; x < static_cast<int>(std::ceil((curPos.x + curRadius) / gridTileSize)); ++x)
		{
			for (int y{ static_cast<int>((curPos.y - curRadius) / gridTileSize) }; y < static_cast<int>(std::ceil((curPos.y + curRadius) / gridTileSize)); ++y)
			{
				const Vector2 testPos{ static_cast<float>(x * gridTileSize),  static_cast<float>(y * gridTileSize) };
				if ((testPos - curPos).MagnitudeSquared() <= (curRadius + gridTileSize * 1.5f) * (curRadius + gridTileSize * 1.5f))
				{
					m_pInfluenceGrid->SetInfluenceAtPosition(testPos, enemyInfluence);
				}
			}
		}
	}

	// Update the food positions on the influence grid
	for (AgarioFood* pFood : m_pFoodVec)
	{
		m_pInfluenceGrid->SetInfluenceAtPosition(pFood->GetPosition(), 75);
	}

	// Propagate influence on the grid
	m_pInfluenceGrid->PropagateInfluence(deltaTime);

	// Set the target of the player
	const Vector2 playerPos{ m_pCustomAgent->GetPosition() };
	const float playerRadius{ m_pCustomAgent->GetRadius() };
	const float foodTestRadius{ gridTileSize * 5 };
	const float enemyTestRadius{ gridTileSize * 2 };
	Vector2 seekTarget{};
	Vector2 fleeTarget{};
	float maxInfluence{ -1000 };
	float minInfluence{ 1000 };
	const float fleeThreshold{ -40.0f };
	const float worldSizeSqr{ m_TrimWorldSize * m_TrimWorldSize };
	for (int x{ max(static_cast<int>((playerPos.x - playerRadius - foodTestRadius) / gridTileSize), 0) }; x < min(static_cast<int>(std::ceil((playerPos.x + playerRadius + foodTestRadius) / gridTileSize)), m_GridSize - 1); ++x)
	{
		for (int y{ max(static_cast<int>((playerPos.y - playerRadius - foodTestRadius) / gridTileSize), 0) }; y < min(static_cast<int>(std::ceil((playerPos.y + playerRadius + foodTestRadius) / gridTileSize)), m_GridSize - 1); ++y)
		{
			const Vector2 testPos{ static_cast<float>(x * gridTileSize) + gridTileSize / 2.0f,  static_cast<float>(y * gridTileSize) + gridTileSize / 2.0f };
			const float distance{ testPos.DistanceSquared(playerPos) };
			float testInfluence{ m_pInfluenceGrid->GetNodeAtWorldPos(testPos)->GetInfluence() };
			if (testInfluence * (worldSizeSqr - distance) > maxInfluence)
			{
				maxInfluence = testInfluence * (worldSizeSqr - distance);
				seekTarget = testPos;
			}
			if (distance < (enemyTestRadius + playerRadius) * (enemyTestRadius + playerRadius) && testInfluence < minInfluence)
			{
				minInfluence = testInfluence;
				fleeTarget = testPos;
			}
		}
	}
	if (minInfluence >= fleeThreshold)
	{
		m_pCustomAgent->SetToSeek(seekTarget);
		DEBUGRENDERER2D->DrawCircle(seekTarget, 2.0f, Color{ 1.0f, 0.0f, 0.0f }, DEBUGRENDERER2D->NextDepthSlice());
	}
	else
	{
		m_pCustomAgent->SetToFlee(fleeTarget);
		DEBUGRENDERER2D->DrawCircle(fleeTarget, 2.0f, Color{ 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());
	}

	std::vector<Vector2> foodRadius
	{
			Vector2{ (playerPos.x - playerRadius - foodTestRadius),(playerPos.y - playerRadius - foodTestRadius) },
			Vector2{ (playerPos.x - playerRadius - foodTestRadius),(playerPos.y + playerRadius + foodTestRadius) },
			Vector2{ (playerPos.x + playerRadius + foodTestRadius),(playerPos.y + playerRadius + foodTestRadius) },
			Vector2{ (playerPos.x + playerRadius + foodTestRadius),(playerPos.y - playerRadius - foodTestRadius) }
	};
	DEBUGRENDERER2D->DrawPolygon(foodRadius.data(), 4, Color{0.0f, 1.0f, 0.0f}, DEBUGRENDERER2D->NextDepthSlice());

	std::vector<Vector2> enemyRadius
	{
			Vector2{ (playerPos.x - playerRadius - enemyTestRadius),(playerPos.y - playerRadius - enemyTestRadius) },
			Vector2{ (playerPos.x - playerRadius - enemyTestRadius),(playerPos.y + playerRadius + enemyTestRadius) },
			Vector2{ (playerPos.x + playerRadius + enemyTestRadius),(playerPos.y + playerRadius + enemyTestRadius) },
			Vector2{ (playerPos.x + playerRadius + enemyTestRadius),(playerPos.y - playerRadius - enemyTestRadius) }
	};
	DEBUGRENDERER2D->DrawPolygon(enemyRadius.data(), 4, Color{ 0.0f, 1.0f, 0.0f }, DEBUGRENDERER2D->NextDepthSlice());
}

void App_AgarioGame_IM::Render(float deltaTime) const
{
	RenderWorldBounds(m_TrimWorldSize);

	m_pInfluenceGrid->SetNodeColorsBasedOnInfluence();
	m_GraphRenderer.RenderGraph(m_pInfluenceGrid, true, false, false, true);

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

Blackboard* App_AgarioGame_IM::CreateBlackboard(AgarioAgent* a)
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

void App_AgarioGame_IM::UpdateImGui()
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
