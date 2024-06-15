#pragma once

#include <cassert>
#include <map>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class CameraSim
{
public:
	enum class Mode
	{
		Fly,
		Walk
	};

	enum class Direction
	{
		Forward,
		Backward,
		Left,
		Right,
		Up,
		Down,
		Nowhere
	};


private:
	constexpr static size_t realDirectionAmount = 6;

	int windowHeight;
	int windowWidth;
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 front;
	glm::vec3 pos;
	glm::vec3 lastPos;
	bool lastBlocker;
	float fov;
	float speed;

	// later can add other camera configurations
	// besides regular fps camera
	float lastX;
	float lastY;
	float yaw;
	float pitch;
	float sensitivity; 

	Mode mode;
	Direction lastDir;

	std::map<Direction, bool> disabledDirs;
public:


	CameraSim(
		const int windowHeight,
		const int windowWidth,
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, -1.0f),
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const float fov = 45.0f,
		const float speed = 0.006f
	)
		: windowHeight(windowHeight), windowWidth(windowWidth), front(front), pos(pos), fov(fov), speed(speed)
	{
		view = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(fov), static_cast<float>(windowHeight / windowWidth), 0.1f, 100.f);

		lastX = static_cast<float>(windowHeight);
		lastY = static_cast<float>(windowWidth);
		yaw = -90.f;
		pitch = 0.0f;
		sensitivity = 0.05f;

		lastBlocker = false;

		mode = Mode::Fly;

		for (size_t i = 0; i < realDirectionAmount; ++i)
		{
			disabledDirs[static_cast<Direction>(i)] = false;
		}
	}

	void InvertMovement()
	{
		speed = (-1) * speed;
	}

	bool IsMovementInverted()
	{
		return speed < 0.0f;
	}

	glm::mat4& GetViewMatrixAddr()
	{
		return view;
	}

	glm::mat4& GetProjectionMatrixAddr()
	{
		return projection;
	}

	glm::vec3& GetFrontVectorAddr()
	{
		return front;
	}

	glm::vec3& GetPositionVectorAddr()
	{
		return pos;
	}

	void DisableDirection(Direction dir)
	{
		disabledDirs[dir] = true;
	}

	void EnableDirection(Direction dir)
	{
		disabledDirs[dir] = false;
	}

	void EnableAllDirections()
	{
		for (size_t i = 0; i < realDirectionAmount; ++i)
		{
			disabledDirs[static_cast<Direction>(i)] = false;
		}
	}

	size_t GetAmountOfDisabledDirs()
	{
		size_t amount = 0;

		for (size_t i = 0; i < realDirectionAmount; ++i)
		{
			amount += disabledDirs[static_cast<Direction>(i)];
		}

		return amount;
	}

	Direction GetLastDirection()
	{
		return lastDir;
	}

	void SetLastPosition()
	{
		pos = lastPos;
	}

	void SetPosition(Direction dir)
	{
		if (disabledDirs[dir])
		{
			return;
		}

		if (!lastBlocker && dir != Direction::Nowhere)
		{
			lastBlocker = true;
			lastDir = dir;
			lastPos = pos;
		}
		else
		{
			lastBlocker = false;
		}

		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 limitY(1.0f, static_cast<float>(mode == Mode::Fly), 1.0f);

		switch (dir)
		{
		case Direction::Forward:
			pos += speed * front * limitY;
			break;
		case Direction::Backward:
			pos -= speed * front * limitY;
			break;
		case Direction::Left:
			pos -= speed * glm::cross(front, up);
			break;
		case Direction::Right:
			pos += speed * glm::cross(front, up);
			break;
		case Direction::Nowhere:
			break;
		case Direction::Up:
		case Direction::Down:
		default:
			assert(false && "Undefined direction");
			return;
		}

		view = glm::lookAt(pos, pos + front, up);
		projection = glm::perspective(glm::radians(fov), static_cast<float>(windowHeight / windowWidth), 0.1f, 100.f);
	}

	void Rotate(float xpos, float ypos)
	{
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch >= 90.0f)
		{
			pitch = 90.0f;
		}
		else if (pitch <= -90.0f)
		{
			pitch = -90.0f;
		}

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(direction);
	}

	void Zoom(float xpos, float ypos)
	{
		if (fov + ypos > 1.0f && fov - ypos < 45.0f)
		{
			fov -= ypos;
		}
		else if (fov <= 1.0f)
		{
			fov = 1.0f;
		}
		else if (fov >= 45.0f)
		{
			fov = 45.0f;
		}
	}
	
	void SetMode(Mode mode)
	{
		this->mode = mode;
	}
};