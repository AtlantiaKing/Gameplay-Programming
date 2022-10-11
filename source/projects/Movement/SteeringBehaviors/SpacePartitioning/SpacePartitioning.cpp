#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_CellWidth{ width / cols }
	, m_CellHeight{ height / rows }
{
	for (int y{}; y < m_NrOfRows; ++y)
	{
		for (int x{}; x < m_NrOfCols; ++x)
		{
			m_Cells.push_back(Cell{ x * m_CellWidth, y * m_CellHeight, m_CellWidth, m_CellHeight });
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	const int cellIdx{ PositionToIndex(agent->GetPosition()) };
	m_Cells[cellIdx].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	const int prevCellIdx{ PositionToIndex(oldPos) };
	const int curCellIdx{ PositionToIndex(agent->GetPosition()) };

	if (curCellIdx == prevCellIdx) return;

	m_Cells[curCellIdx].agents.push_back(agent);
	m_Cells[prevCellIdx].agents.remove(agent);
}

int CellSpace::RegisterNeighbors(std::vector<SteeringAgent*>& pNeigbors, SteeringAgent* pAgent, float queryRadius)
{
	int nrOfNeighbors{};

	const int centerCell{ PositionToIndex(pAgent->GetPosition()) };
	const Elite::Vector2 curPos{ pAgent->GetPosition() };

	const int minX{ static_cast<int>((curPos.x - queryRadius) / m_CellWidth) };
	const int minY{ static_cast<int>((curPos.y - queryRadius) / m_CellWidth) };
	const int maxX{ static_cast<int>((curPos.x + queryRadius) / m_CellWidth) };
	const int maxY{ static_cast<int>((curPos.y + queryRadius) / m_CellWidth) };

	for (int y{ minY < 0 ? 0 : minY }; y <= maxY && y < m_NrOfCols; ++y)
	{
		if (y < 0) continue;

		for (int x{ minX < 0 ? 0 : minX }; x <= maxX && x < m_NrOfRows; ++x)
		{

			const Cell& pCell{ m_Cells[y * m_NrOfRows + x] };
			const std::list<SteeringAgent*>& agents{ pCell.agents };

			for (SteeringAgent* pOtherAgent : agents)
			{
				if (pOtherAgent == pAgent) continue;

				const float sqrDistance{ (pOtherAgent->GetPosition() - pAgent->GetPosition()).MagnitudeSquared() };

				if (sqrDistance < queryRadius * queryRadius)
				{
					pNeigbors[nrOfNeighbors++] = pOtherAgent;

					if (pAgent->CanRenderBehavior())
						DEBUGRENDERER2D->DrawSolidCircle(
							pOtherAgent->GetPosition(),
							pOtherAgent->GetRadius(),
							pOtherAgent->GetLinearVelocity().GetNormalized(),
							{ 0.0f, 1.0f, 0.0f },
							0.0f
						);
				}
			}

			if (pAgent->CanRenderBehavior())
			{
				const std::vector<Elite::Vector2> rectPoints{ pCell.GetRectPoints() };

				DEBUGRENDERER2D->DrawPolygon(rectPoints.data(), rectPoints.size(), Elite::Color{ 0.0f, 0.0f, 1.0f }, 0.0f);
			}
		}
	}
	return nrOfNeighbors;
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	const Elite::Color color{ 1.0f, 0.0f, 0.0f };

	for (Cell pCell : m_Cells)
	{
		const std::vector<Elite::Vector2> rectPoints{ pCell.GetRectPoints() };

		DEBUGRENDERER2D->DrawPolygon(rectPoints.data(), rectPoints.size(), color, 0.0f);

		DEBUGRENDERER2D->DrawString(Elite::Vector2{ pCell.boundingBox.bottomLeft.x + pCell.boundingBox.width / 2.0f, pCell.boundingBox.bottomLeft.y + pCell.boundingBox.height / 2.0f }, std::to_string(pCell.agents.size()).c_str());
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	int x{ int(pos.x / m_CellWidth) };
	int y{ int(pos.y / m_CellHeight) };

	if (x >= m_NrOfCols) x = m_NrOfCols - 1;
	if (y >= m_NrOfRows) y = m_NrOfRows - 1;

	return y * m_NrOfRows + x;
}