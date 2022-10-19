#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include "../SpacePartitioning/SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 10.0f }
	, m_NrOfNeighbors{0}
{
	// Create each behavior
	m_pSeekBehavior = new Seek{};
	m_pSeparationBehavior = new Separation{ this };
	m_pCohesionBehavior = new Cohesion{ this };
	m_pVelMatchBehavior = new VelocityMatch{ this };
	m_pWanderBehavior = new Wander{};
	m_pWanderBehavior->SetMaxAngleChange(0.01f);
	m_pEvadeBehavior = new Evade{};
	m_pEvadeBehavior->SetFleeRadius(20.0f);

	// The weight of each behavior in the blended behavior (there are 5 behaviors in our example)
	const float weightBehaviour{ 1.0f / 5.0f };

	// Create the blended behavior
	m_pBlendedSteering = new BlendedSteering{ { 
		{ m_pSeekBehavior, 0.2f/*weightBehaviour*/ },
		{ m_pSeparationBehavior, 0.4f/*weightBehaviour*/ },
		{ m_pCohesionBehavior, 0.2f/*weightBehaviour*/ },
		{ m_pVelMatchBehavior, 0.4f/*weightBehaviour*/ },
		{ m_pWanderBehavior, 0.4f/*weightBehaviour*/ }
			} };

	// Create the priority behavior
	m_pPrioritySteering = new PrioritySteering{ { m_pEvadeBehavior, m_pBlendedSteering } };

	// Resize the agent container to the amount of agents
	m_pAgents.resize(m_FlockSize);

	m_pCellSpace = new CellSpace(m_WorldSize, m_WorldSize, m_NrPartitionsInAxis, m_NrPartitionsInAxis);

	// Initialize each agent in the flock
	for (int i = 0; i < m_FlockSize; ++i)
	{
		SteeringAgent* pNewAgent{ new SteeringAgent{} };

		pNewAgent->SetSteeringBehavior(m_pPrioritySteering);

		pNewAgent->SetAutoOrient(true);
		pNewAgent->SetPosition({ rand() % 1001 / 1000.0f * m_WorldSize, rand() % 1001 / 1000.0f * m_WorldSize });
		pNewAgent->SetMaxLinearSpeed(50.0f);
		pNewAgent->SetMass(1.0f);

		m_pAgents[i] = pNewAgent;

		if (i == m_FlockSize - 1)
		{
			int i = 0;
		}

		m_pCellSpace->AddAgent(pNewAgent);
	}

	// Resize the neighbor container to the amount of agents except one (the current agent is never included)
	m_pNeighbors.resize(m_FlockSize - 1);
}

Flock::~Flock()
{
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior);

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	SAFE_DELETE(m_pCellSpace);

	for(auto pAgent: m_pAgents)
	{
		SAFE_DELETE(pAgent);
	}
	m_pAgents.clear();
	m_pNeighbors.clear();
}

void Flock::Update(float deltaT)
{
	auto target = TargetData{};
	target.Position = m_pAgentToEvade->GetPosition();
	target.Orientation = m_pAgentToEvade->GetRotation();
	target.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	target.AngularVelocity = m_pAgentToEvade->GetAngularVelocity();
	m_pEvadeBehavior->SetTarget(target);

	for (SteeringAgent* pAgent : m_pAgents)
	{
		const Vector2 prevPosition{ pAgent->GetPreviousPosition() };

		if (m_TrimWorld)
		{
			pAgent->TrimToWorld(m_WorldSize);
		}

		if (m_IsUsingPartitioning)
		{
			m_pCellSpace->UpdateAgentCell(pAgent, prevPosition);
			m_NrOfNeighbors = m_pCellSpace->RegisterNeighbors(m_pNeighbors, pAgent, m_NeighborhoodRadius);
		}
		else
		{
			RegisterNeighbors(pAgent);
		}

		pAgent->Update(deltaT);
	}
}

void Flock::Render(float deltaT)
{
	//m_pAgentToEvade->Render(deltaT);

	for (SteeringAgent* pAgent : m_pAgents)
	{
		if (m_IsRenderingAgents) pAgent->Render(deltaT);
		if (pAgent->CanRenderBehavior())
		{
			DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_NeighborhoodRadius, { 0.0f, 1.0f, 0.0f }, 0.0f);
		}
	}

	if (m_IsUsingPartitioning && m_DrawPartitioning)
	{
		m_pCellSpace->RenderCells();
	}
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
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

	ImGui::Text("Flocking");
	ImGui::Spacing();

	if (ImGui::Checkbox("Use Space Partitioning", &m_IsUsingPartitioning))
	{
		if(m_IsUsingPartitioning) m_pCellSpace->EmptyCells();
	}

	ImGui::Checkbox("Is rendering agents", &m_IsRenderingAgents);

	// Display debug logic
	if (ImGui::CollapsingHeader("Debug Info"))
	{
		ImGui::Checkbox("Draw Space Partitioning", &m_DrawPartitioning);

		bool isCheckedWander = m_pAgentToEvade->CanRenderBehavior();
		ImGui::Checkbox("Render Wandering Agent Debug", &isCheckedWander);
		m_pAgentToEvade->SetRenderBehavior(isCheckedWander);

		ImGui::Checkbox("Render Neighborhood Debug", &m_IsDebugingAgent);
		m_pAgents[m_AgentToDebug]->SetRenderBehavior(m_IsDebugingAgent);

		int prevDebugAgent = m_AgentToDebug;
		ImGui::SliderInt("Debug Agent", &m_AgentToDebug, 0, m_pAgents.size() - 1);
		if (prevDebugAgent != m_AgentToDebug) m_pAgents[prevDebugAgent]->SetRenderBehavior(false);
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Display properties from the current agent
	if (ImGui::CollapsingHeader("Flock Properties"))
	{
		auto v = m_pAgents[0]->GetMaxLinearSpeed();
		if (ImGui::SliderFloat("Lin", &v, 0.f, 50.0f, "%.2f"))
		{
			for (SteeringAgent* pAgent : m_pAgents)
			{
				pAgent->SetMaxLinearSpeed(v);
			}
		}

		v = m_pAgents[0]->GetMaxAngularSpeed();
		if (ImGui::SliderFloat("Ang", &v, 0.f, 35.f, "%.2f"))
		{
			for (SteeringAgent* pAgent : m_pAgents)
			{
				pAgent->SetMaxAngularSpeed(v);
			}
		}

		v = m_pAgents[0]->GetMass();
		if (ImGui::SliderFloat("Mass ", &v, 0.f, 20.f, "%.2f"))
		{
			for (SteeringAgent* pAgent : m_pAgents)
			{
				pAgent->SetMass(v);
			}
		}

		ImGui::SliderFloat("Neighborhood radius", &m_NeighborhoodRadius, 5.0f, 20.0f, "%0.2f");
	}

	ImGui::Spacing();

	// Display weights of all the behaviours in the flock
	if (ImGui::CollapsingHeader("Flock Weights"))
	{
		ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
		ImGui::SliderFloat("Seperation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
		ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
		ImGui::SliderFloat("Vel Match", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
		ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Display properties from the wandering agent
	if (ImGui::CollapsingHeader("Wandering Agent Properties"))
	{
		auto v = m_pAgentToEvade->GetMaxLinearSpeed();
		if (ImGui::SliderFloat("Lin", &v, 0.f, 35.f, "%.2f"))
			m_pAgentToEvade->SetMaxLinearSpeed(v);

		v = m_pAgentToEvade->GetMaxAngularSpeed();
		if (ImGui::SliderFloat("Ang", &v, 0.f, 35.f, "%.2f"))
			m_pAgentToEvade->SetMaxAngularSpeed(v);

		v = m_pAgentToEvade->GetMass();
		if (ImGui::SliderFloat("Mass ", &v, 0.f, 20.f, "%.2f"))
			m_pAgentToEvade->SetMass(v);
	}

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	m_NrOfNeighbors = 0;
	for (SteeringAgent* pOtherAgent : m_pAgents)
	{
		if (pOtherAgent == pAgent) continue;

		float distance{ (pAgent->GetPosition() - pOtherAgent->GetPosition()).Magnitude() };
		if (distance < m_NeighborhoodRadius)
		{
			m_pNeighbors[m_NrOfNeighbors++] = pOtherAgent;

			if(pAgent->CanRenderBehavior())
				DEBUGRENDERER2D->DrawSolidCircle(
					pOtherAgent->GetPosition(), 
					pOtherAgent->GetRadius(), 
					pOtherAgent->GetLinearVelocity().GetNormalized(), 
					{ 0.0f, 1.0f, 0.0f }, 
					0.0f
				);
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	// If there are no neighbors, return an empty position
	if (m_NrOfNeighbors == 0) return Vector2{};
	
	// Calculate the average of all the neighor position
	Vector2 averageNeighborPos{};
	for (int i{}; i < m_NrOfNeighbors; ++i)
	{
		averageNeighborPos += m_pNeighbors[i]->GetPosition();
	}
	averageNeighborPos /= static_cast<float>(m_NrOfNeighbors);

	return averageNeighborPos;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	if (m_NrOfNeighbors == 0) return Vector2{};

	Vector2 averageNeighborVel{};
	for (int i{}; i < m_NrOfNeighbors; ++i)
	{
		averageNeighborVel += m_pNeighbors[i]->GetLinearVelocity();
	}
	averageNeighborVel /= static_cast<float>(m_NrOfNeighbors);

	return averageNeighborVel;
}

float Flock::GetNeighborhoodRadius() const
{
	return m_NeighborhoodRadius;
}

void Flock::SetTarget_Seek(TargetData target)
{
	m_pSeekBehavior->SetTarget(target);
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
