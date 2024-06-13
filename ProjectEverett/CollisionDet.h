#pragma once 

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


class CollisionDet
{
public:
	struct ObjectInfo
	{
		glm::vec3 coords;
		glm::vec3 scale;
	};


	static bool CheckForCollision(const ObjectInfo& object1, const ObjectInfo& object2)
	{
		bool res = true;

		for (int i = 0; i <3; ++i)
		{
			res = ((object1.coords[i] + object1.scale[i] / 2) - (object2.coords[i] - object2.scale[i] / 2)) > 0.0f &&
				  ((object1.coords[i] - object1.scale[i] / 2) - (object2.coords[i] + object2.scale[i] / 2)) < 0.0f;
			
			if (!res) break;
		}

		return res;
	}
};
