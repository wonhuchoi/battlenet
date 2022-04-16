#include "game_components.hpp"
#include "board.hpp"
#include "cell.hpp"
#include "render.hpp"
#include "tiny_ecs.hpp"
#include <iostream> 
#include <vector> 

ECS::Entity Board::createBoard(vec2 wp)
{
	auto entity = ECS::Entity();

	// Setting WorldPosition component
	WorldPosition& worldPosition = ECS::registry<WorldPosition>.emplace(entity);
	worldPosition.position = wp;

	ECS::registry<BoardInfo>.emplace(entity);
	BoardInfo& boardInfo = ECS::registry<BoardInfo>.get(entity);
	vec2 cellSize = boardInfo.cellSize;
	// Create a new sprite for each cell starting from the upper-left corner
	int numCols = boardInfo.boardSize.x;
	int numRows = boardInfo.boardSize.y;
	ECS::registry<Board>.emplace(entity);
	ECS::Entity** grid = new ECS::Entity * [numRows];
	for (int i = 0; i < numRows; i++) {
		ECS::Entity* currRow = new ECS::Entity[numCols];
		for (int j = 0; j < numCols + 1; j++) {
			CellType cellType;
			if (i < numRows / 2)
				cellType = enemyCell;
			else
				cellType = allyCell;
			currRow[j] = Cell::createCell({
					j, i
				},
				{ 
					j * cellSize.x + wp.x,
					i * cellSize.y + wp.y,
				}, cellSize,
				cellType);
			}
		grid[i] = currRow;
	}

	ECS::Entity* bench = new ECS::Entity[numCols];
	for (int j = 0; j < numCols - 1; j++) {
		bench[j] = Cell::createCell(
			{ j, numRows },
			{ j * cellSize.x + wp.x,
				numRows * cellSize.y + wp.y,
			}, cellSize,
			benchCell);
	}
	bench[numCols - 1] = Cell::createCell(
		{ (numCols - 1), numRows },
		{ (numCols - 1) * cellSize.x + wp.x,
			numRows * cellSize.y + wp.y,
		}, cellSize,
		shopCell);
	grid[numRows] = bench;
	boardInfo.cells = grid;
	return entity;
}

ECS::Entity* Board::getBench(ECS::Entity board)
{
	BoardInfo& boardInfo = ECS::registry<BoardInfo>.get(board);
	int bench_index = boardInfo.boardSize.y;
	return boardInfo.cells[bench_index];
}

ECS::Entity Board::getCell(ECS::Entity board, vec2 pos)
{
	BoardInfo& boardInfo = ECS::registry<BoardInfo>.get(board);
	int x = pos.x;
	int y = pos.y;
	return boardInfo.cells[x][y];
}

ECS::Entity** Board::getCells(ECS::Entity board)
{
	BoardInfo& boardInfo = ECS::registry<BoardInfo>.get(board);
	return boardInfo.cells;
}



void Board::hide(ECS::Entity board)
{
    BoardInfo& boardInfo = ECS::registry<BoardInfo>.get(board);
    for (int i = 0; i < boardInfo.boardSize.x; i++) {
        for (int j = 0; j < boardInfo.boardSize.y+1; j++) {
            Cell::hide(boardInfo.cells[i][j]);
        }
    }
}

void Board::show(ECS::Entity board)
{
	BoardInfo& boardInfo = ECS::registry<BoardInfo>.get(board);
	for (int i = 0; i < boardInfo.boardSize.y+1; i++) {
		for (int j = 0; j < boardInfo.boardSize.x; j++) {
			Cell::show(boardInfo.cells[i][j]);
		}
	}
}
