#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			const Triangle* startTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos) };
			const Triangle* endTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos) };

			//We have valid start/end triangles and they are not the same
			if (!startTriangle || !endTriangle)
			{
				std::cout << "Start/End position must be on Navmesh\n";
				return finalPath;
			}

			if (startTriangle == endTriangle)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}
		
			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto graphCopy{ pNavGraph->Clone() };
			
			//Create extra node for the Start Node (Agent's position)
			NavGraphNode* startNode{ new NavGraphNode { graphCopy->GetNextFreeNodeIndex(), -1, startPos}};
			graphCopy->AddNode(startNode);
			for (int edgeIdx : startTriangle->metaData.IndexLines)
			{
				const int targetNodeIdx{ pNavGraph->GetNodeIdxFromLineIdx(edgeIdx) };
				if (targetNodeIdx == invalid_node_index) continue;

				graphCopy->AddConnection(new GraphConnection2D{ startNode->GetIndex(), targetNodeIdx, startPos.Distance(graphCopy->GetNode(targetNodeIdx)->GetPosition()) });
			}
			
			//Create extra node for the endNode
			NavGraphNode* endNode{ new NavGraphNode { graphCopy->GetNextFreeNodeIndex(), endPos } };
			graphCopy->AddNode(endNode);
			for (int edgeIdx : endTriangle->metaData.IndexLines)
			{
				const int targetNodeIdx{ pNavGraph->GetNodeIdxFromLineIdx(edgeIdx) };
				if (targetNodeIdx == invalid_node_index) continue;

				graphCopy->AddConnection(new GraphConnection2D{ endNode->GetIndex(), targetNodeIdx, endPos.Distance(graphCopy->GetNode(targetNodeIdx)->GetPosition()) });
			}
			
			//Run A star on new graph
			auto pathfinder = AStar<NavGraphNode, GraphConnection2D>(graphCopy.get(), Elite::HeuristicFunctions::Chebyshev);
			const std::vector<NavGraphNode*> pathInNodes{ pathfinder.FindPath(startNode, endNode) };

			// Debug non optimized path
			debugNodePositions.clear();
			for (NavGraphNode* pathNode : pathInNodes)
			{
				const Vector2 currentNodePosition{ pathNode->GetPosition() };

				debugNodePositions.push_back(currentNodePosition);
				finalPath.push_back(currentNodePosition);
			}

			// Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			auto portals = SSFA::FindPortals(pathInNodes, pNavGraph->GetNavMeshPolygon());

			// Debug portals
			debugPortals = portals;

			finalPath = SSFA::OptimizePortals(portals);

			return finalPath;
		}
	};
}
