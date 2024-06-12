#include <iostream>
#include <fstream>
#include <cstdlib>
#include <time.h>

#include <Windows.h>

#include "MazeGen.h"

MazeGen::MazeInfo MazeGen::GenerateMaze(size_t n, size_t m, bool onlySolvable)
{
	MazeInfo maze{};
	MazeGen mazeGenInst;

	srand(static_cast<unsigned int>(time(nullptr)));

	mazeGenInst.ResizeAndFillMatrix(maze.matrix, n, m);
	do
	{
		mazeGenInst.RandomizeMaze(maze);
		mazeGenInst.SetStartAndEnd(maze);
	} while (!mazeGenInst.IsSolvable(maze) && onlySolvable);

	return maze;
}

bool MazeGen::IsSolvable(MazeInfo& maze)
{
	using ll = long long;

	ll matrixN = static_cast<ll>(maze.matrix.size());
	ll matrixM = static_cast<ll>(maze.matrix[0].size());

	std::vector<std::vector<bool>> visitedMatrix;
	ResizeAndFillMatrix(visitedMatrix, matrixN, matrixM);

	bool search = true;
	bool found = false;

	bool pathFound = false;

	ll n = 0;
	ll k = 0;
	while (search)
	{
		visitedMatrix[n][k] = true;
		pathFound = false;

		if (n == matrixN - 1 && k == matrixM - 1)
		{
			search = false;
			found = true;
		}
		else if (k + 1 < matrixM && maze.matrix[n][k + 1] && !visitedMatrix[n][k + 1])
		{
			++k;
			maze.path.push_back(Direction::Right);
		}
		else if (n + 1 < matrixN && maze.matrix[n + 1][k] && !visitedMatrix[n + 1][k])
		{
			++n;
			maze.path.push_back(Direction::Down);
		}
		else if (k - 1 >= 0 && maze.matrix[n][k - 1] && !visitedMatrix[n][k - 1])
		{
			--k;
			maze.path.push_back(Direction::Left);
		}
		else if (n - 1 >= 0 && maze.matrix[n - 1][k] && !visitedMatrix[n - 1][k])
		{
			--n;
			maze.path.push_back(Direction::Up);
		}
		else
		{
			if (n == 0 && k == 0) search = false;
			else
			{
				Direction lastPoint = maze.path.back();

				switch (lastPoint)
				{
				case Direction::Right:
					--k;
					break;
				case Direction::Left:
					++k;
					break;
				case Direction::Down:
					--n;
					break;
				case Direction::Up:
					++n;
					break;
				default:
					break;
				}

				maze.path.pop_back();
			}
		}
	}

	if (!found)
	{
		maze.path.clear();
	}

	return found;
}

void MazeGen::SetStartAndEnd(MazeInfo& maze)
{
	maze.matrix.front().front() = true;
	maze.matrix.back().back()   = true;
}

void MazeGen::ResizeAndFillMatrix(std::vector<std::vector<bool>>& matrix, size_t N, size_t M)
{
	matrix.resize(N, std::vector<bool>(M, false));
}

void MazeGen::RandomizeMaze(MazeInfo& maze)
{
	for (int i = 0; i < maze.matrix.size(); i++)
	{
		for (int j = 0; j < maze.matrix[i].size(); j++)
		{
			maze.matrix[i][j] = rand() % 2;
		}
	}
}

bool MazeGen::CheckIfMazeIsValidInFile(const std::string& filename, size_t& N, size_t& M)
{
	std::ifstream file(filename);
	char currentChar;

	bool correctFile = true;

	do 
	{
		file.get(currentChar);
		if (currentChar == '0' || currentChar == '1') ++M;
		else if (currentChar == ' ') continue;
		else if (currentChar == '\n') ++N;
		else
		{
			correctFile = false;
			break;
		}
	} while (!file.eof());
	file.close();

	return correctFile;
}

bool MazeGen::ReadMazeFromFile(MazeInfo& maze, const std::string& filename)
{
	std::ifstream file(filename);
	char currentChar;
	size_t i = 0;
	size_t j = 0;

	while (!file.eof())
	{
		file.get(currentChar);
		if (currentChar == '0' || currentChar == '1')
		{
			maze.matrix[i][j] = currentChar - '0';
			++j;
		}
		else if (currentChar == ' ') continue;
		else if (currentChar == '\n')
		{
			++i;
			j = 0;
		}
	}
	file.close();

	return true;
}

MazeGen::MazeInfo MazeGen::LoadMazeFromFile(const std::string& filename)
{
	MazeInfo maze{};
	MazeGen mazeGenInst;

	size_t N = 0;
	size_t M = 0;

	if (!CheckIfMazeIsValidInFile(filename, N, M))
	{
		std::cout << "Matrix in the file is not valid\n";
	}
	else
	{
		--N;
		M /= N;
		mazeGenInst.ResizeAndFillMatrix(maze.matrix, N, M);
		mazeGenInst.ReadMazeFromFile(maze, filename);
		mazeGenInst.IsSolvable(maze);
	}

	return maze;
}

bool MazeGen::SaveMazeToFile(const MazeInfo& maze, const std::string& filename)
{
	std::ofstream file(filename, std::ios::out);

	while (!file.eof())
	{
		for (size_t i = 0; i < maze.matrix.size(); ++i)
		{
			for (size_t j = 0; j < maze.matrix[i].size(); ++j)
			{
				file << maze.matrix[i][j] << ' ';
			}
			file << '\n';
		}
	}
	file.close();


	return true;
}

void MazeGen::PrintExitPath(MazeInfo& maze) 
{
	if (!maze.path.size())
	{
		std::cerr << "Maze is unsolvable\n";
	}

	MazeGen mazeGenInst;

	int direction[4][2] = {
		{ 0,  1},
		{ 1,  0},
		{ 0, -1},
		{-1,  0}
	};

	size_t N = maze.matrix.size();
	size_t M = maze.matrix[0].size();

	std::vector<std::vector<bool>> realPathMatrix;
	mazeGenInst.ResizeAndFillMatrix(realPathMatrix, N, M);

	int currentPath[] = { 0, 0 };

	for (const Direction dir : maze.path) 
	{
		realPathMatrix[currentPath[0]][currentPath[1]] = true;

		for (short j = 0; j < 4; j++) 
		{
			if (static_cast<Direction>(j) == dir)
			{
				currentPath[0] += direction[j][0];
				currentPath[1] += direction[j][1];
				break;
			}
		}
	}

	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	realPathMatrix.front().front() = true;
	realPathMatrix.back().back() = true;
	for (size_t i = 0; i < N; ++i) 
	{
		for (size_t j = 0; j < M; ++j) 
		{
			if (realPathMatrix[i][j])
			{
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
			}
			else SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
			std::cout << maze.matrix[i][j] << ' ';
		}
		std::cout << '\n';
	}
	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void MazeGen::PrintExitPath(MazeInfo&& maze)
{
	PrintExitPath(maze);
}
