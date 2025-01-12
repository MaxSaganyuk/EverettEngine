#include "EverettScript.h"

ScriptFunc(Rise)
{
    glm::vec3& pos = solid.GetPositionVectorAddr();
    solid.Rotate(ISolidSim::Rotation{ 0, 0.0001, 0 });

    solid.ForceModelUpdate();
}