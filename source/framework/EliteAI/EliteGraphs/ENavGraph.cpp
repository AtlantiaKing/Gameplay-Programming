#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	for (Line* edge : m_pNavMeshPolygon->GetLines())
	{
		// If the edge is shared by two triangles, add the center of the edge to the nodes list
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(edge->index).size() > 1)
		{
			AddNode(new NavGraphNode(GetNextFreeNodeIndex(), edge->index, (edge->p1 + edge->p2) / 2));
		}
	}
	
	//2. Create connections now that every node is created
	for (Triangle* triangle : m_pNavMeshPolygon->GetTriangles())
	{
		std::vector<int> validNodeIndexes{};
		for (int edgeIdx : triangle->metaData.IndexLines)
		{
			const int nodeIdx{ GetNodeIdxFromLineIdx(edgeIdx) };
			if (nodeIdx != invalid_node_index)
			{
				validNodeIndexes.push_back(nodeIdx);
			}
		}

		switch (validNodeIndexes.size())
		{
		case 2:
			AddConnection(new GraphConnection2D(validNodeIndexes[0], validNodeIndexes[1]));
			break;
		case 3:
		{
			AddConnection(new GraphConnection2D(validNodeIndexes[0], validNodeIndexes[1]));
			AddConnection(new GraphConnection2D(validNodeIndexes[1], validNodeIndexes[2]));
			AddConnection(new GraphConnection2D(validNodeIndexes[2], validNodeIndexes[0]));
		}
			break;
		}
	}
	
	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();
}

