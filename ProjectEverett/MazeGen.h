#pragma once

#include <vector>

class MazeGen
{
public:

	enum class Direction 
	{
		Right,
		Down,
		Left,
		Up,
	};

	/*
	*  matrix describes 2D plane where 1 is available and 0 is unavailable blocks
	*  path describes a way from start to end, if size is 0 no path exists
	*/
	struct MazeInfo
	{
		std::vector<std::vector<bool>> matrix;
		std::vector<Direction> path;
	};

private:
	void ResizeAndFillMatrix(std::vector<std::vector<bool>>& maze, size_t N, size_t M);

	bool IsSolvable(MazeInfo& maze);
	void SetStartAndEnd(MazeInfo& maze);
	void RandomizeMaze(MazeInfo& maze);

	bool ReadMazeFromFile(MazeInfo& maze, const std::string& filename);
	
	static bool CheckIfMazeIsValidInFile(const std::string& filename, size_t& N, size_t& M);

public:

	static MazeInfo GenerateMaze(size_t n, size_t m, bool onlySolvable = true);
	static MazeInfo LoadMazeFromFile(const std::string& filename);
	static bool     SaveMazeToFile(const MazeInfo& maze, const std::string& filename);

	static void PrintExitPath(MazeInfo& maze);
	static void PrintExitPath(MazeInfo&& maze);
};

