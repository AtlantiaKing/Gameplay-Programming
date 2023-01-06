//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_GraphTheory.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EEularianPath.h"

using namespace Elite;
using namespace std;
//Destructor
App_GraphTheory::~App_GraphTheory()
{
	SAFE_DELETE(m_pGraph2D);
}

//Functions
void App_GraphTheory::Start()
{
	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(0, 0));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	m_pGraph2D = new Graph2D<GraphNode2D, GraphConnection2D>{ false };
	m_pGraph2D->AddNode(new GraphNode2D{ 0, {20.0f, 30.0f} });
	m_pGraph2D->AddNode(new GraphNode2D{ 1, {10.0f, 10.0f} });
	m_pGraph2D->AddConnection(new GraphConnection2D{ 0,1 });
}

void App_GraphTheory::Update(float deltaTime)
{
	m_GraphEditor.UpdateGraph(m_pGraph2D);
	m_pGraph2D->SetConnectionCostsToDistance();

	switch (m_CurrentExercise)
	{
	case GraphExercise::EulerPath:
	{
		auto eulerFinder = EulerianPath<GraphNode2D, GraphConnection2D>{ m_pGraph2D };
		Eulerianity eulerianity = eulerFinder.IsEulerian();
		std::vector<GraphNode2D*> eulerianPath = eulerFinder.FindPath(eulerianity);

		switch (eulerianity)
		{
		case Eulerianity::notEulerian:
		{
			std::cout << "not eulerian\n";
			for (GraphNode2D* n : m_pGraph2D->GetAllNodes())
			{
				n->SetColor(Color{ 0.0f, 0.0f, 1.0f });
			}
			break;
		}
		case Eulerianity::semiEulerian:
		{
			std::cout << "semi eulerian\n";
			for (GraphNode2D* n : m_pGraph2D->GetAllNodes())
			{
				if (n == eulerianPath.front() || n == eulerianPath.back())
				{
					n->SetColor(Color{ 0.5f, 0.5f, 0.5f });
				}
				else
				{
					n->SetColor(Color{ 1.0f, 1.0f, 1.0f });
				}
			}
			break;
		}
		case Eulerianity::eulerian:
		{
			std::cout << "eulerian\n";
			for (GraphNode2D* n : m_pGraph2D->GetAllNodes())
			{
				n->SetColor(Color{ 1.0f, 1.0f, 1.0f });
			}
			break;
		}
		}

		break;
	}
	case GraphExercise::ColorGraph:
	{
		for (GraphNode2D* n : m_pGraph2D->GetAllNodes())
		{
			n->SetColorIndex(-1);
		}
		for (GraphNode2D* n : m_pGraph2D->GetAllNodes())
		{
			auto connections = m_pGraph2D->GetNodeConnections(n);

			for (int colorIdx{}; colorIdx < m_MaxNrOfColors; ++colorIdx)
			{
				bool invalidColor{};
				for (GraphConnection2D* c : connections)
				{
					if (m_pGraph2D->GetNode(c->GetTo())->GetColorIndex() == colorIdx)
					{
						invalidColor = true;
						break;
					}
				}
				if (!invalidColor)
				{
					n->SetColorIndex(colorIdx);
					n->SetColor(m_Colors[colorIdx]);
					break;
				}
			}
		}

		break;
	}
	case GraphExercise::MinimumSpanningTree:
	{
		// Collection of trees
		std::vector<std::vector<GraphConnection2D*>> forest{};
		// All connections
		std::vector<GraphConnection2D*> allConnections{};

		// Fill the container of connections
		for (auto& connections : m_pGraph2D->GetAllConnections())
		{
			allConnections.reserve(allConnections.size() + connections.size());
			allConnections.insert(allConnections.end(), connections.begin(), connections.end());
		}

		// Set the color of all the edges and connections to black
		const Color disabledColor{ 0.0f, 0.0f, 0.0f };
		for (GraphConnection2D* connection : allConnections)
		{
			connection->SetColor(disabledColor);
			m_pGraph2D->GetNode(connection->GetFrom())->SetColor(disabledColor);
			m_pGraph2D->GetNode(connection->GetTo())->SetColor(disabledColor);
		}

		// Sort all connections by distance
		std::sort(
			allConnections.begin(),
			allConnections.end(),
			[](GraphConnection2D* first, GraphConnection2D* second)
			{
				return first->GetCost() < second->GetCost();
			});

		// As long as there are connections to be tested
		while (allConnections.size() > 0)
		{
			// Get the shortest connection and remove it from the container
			GraphConnection2D* curConnection{ *allConnections.begin() };
			allConnections.erase(allConnections.begin());

			// If this becomes true, the current connection will not be added
			bool createsLoop{};

			// The index of the tree the current connection connects with
			int connectedTreeIdx{ -1 };

			// For each tree
			for (size_t i{}; i < forest.size(); ++i)
			{
				// Get a reference to the current tree
				const std::vector<GraphConnection2D*>& tree{ forest[i] };

				// Check whether the current edge is connected with the current tree
				// and get which points are connected
				bool connectedToP0{};
				bool connectedToP1{};

				// For each connection in the tree
				for (GraphConnection2D* edgeInTree : tree)
				{
					// Check if a point of the current connection is on an connection of the tree
					bool edgeConnectedToP0{ edgeInTree->GetFrom() == curConnection->GetFrom() || edgeInTree->GetTo() == curConnection->GetFrom() };
					bool edgeConnectedToP1{ edgeInTree->GetFrom() == curConnection->GetTo() || edgeInTree->GetTo() == curConnection->GetTo() };

					if (edgeConnectedToP0) connectedToP0 = true;
					if (edgeConnectedToP1) connectedToP1 = true;
				}

				if (connectedToP0 && connectedToP1)
				{
					// If the connection is connected with both points, return that this connection would make the tree loop
					createsLoop = true;
					// Stop the loop over the forest
					break;
				}
				else if (connectedToP0 || connectedToP1)
				{
					// If the edge is connected with only one point, return that this edge is connected to the tree
					if (connectedTreeIdx == -1)
					{
						// If the edge is not connected to any tree yet, save the index of the current tree
						connectedTreeIdx = static_cast<int>(i);
					}
					else
					{
						// If the edge is already connected to a tree, merge the two trees
						forest[connectedTreeIdx].reserve(forest[connectedTreeIdx].size() + tree.size());
						forest[connectedTreeIdx].insert(forest[connectedTreeIdx].end(), tree.begin(), tree.end());
						// Remove the current tree
						forest[i] = forest[forest.size() - 1];
						forest.pop_back();
						// Stop the loop over the forest
						break;
					}
				}
			}

			// If the current connection makes the tree loop, continue to the next connection
			if (createsLoop) continue;

			if (connectedTreeIdx == -1)
			{
				// If the current connection is not connected to any tree, create a new tree
				forest.push_back({ { curConnection } });
			}
			else
			{
				// If the edge is connected to a tree, add the edge to the tree
				forest[connectedTreeIdx].push_back(curConnection);
			}
		}

		// Set the color of all the connections of all the trees to blue
		const Color connectedColor{ 0.0f, 0.0f, 1.0f };
		for (const std::vector<GraphConnection2D*> tree : forest)
		{
			for (GraphConnection2D* connection : tree)
			{
				// Set the current connection its color
				connection->SetColor(connectedColor);
				// Set the reverse connection its color (otherwise some connections might still render black)
				m_pGraph2D->GetConnection(connection->GetTo(), connection->GetFrom())->SetColor(connectedColor);
			}
		}

		break;
	}
	}

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
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
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

		ImGui::Text("Graph Theory");
		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
	

}

void App_GraphTheory::Render(float deltaTime) const
{
	m_GraphRenderer.RenderGraph(m_pGraph2D, true, true);
}
