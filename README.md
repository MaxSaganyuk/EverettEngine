# ProjectEverett
ProjectEverett is a combination of two projects - Everett (the game engine), LGL (the OpenGL wrapper). For noww Everett is documented currently as a guide (how to use the application) and LGL is documented as an API (how to code with), this might change later.

Document is accurate for 09.05.2025 version of the code
## Everett
Everett is a minimalistic WIP 3D game engine. Right now uses MFC for GUI, but since it's separated, can be reworked to use other GUI lib. Uses LGL for rendering. Uses Assimp for model loading with animation info and OpenAL for spatial sound

Main windows looks like so. Window on the left is a render window, on the right - interactable GUI:
![image](https://github.com/user-attachments/assets/7c803139-03ca-418a-98e6-2152b949df8b)

Features involve
- Object placing (Model loading and solid, light and sound placing in virtual space)
- Scripting in C++ with DLL hot loading for objects and keybinds
- Unlimited animation playback with adaptive internal shader
- World saving and loading
### Object placing
You can load a model from `obj`, `glb`, `dae`, `fdx` file types and place them as a solid in space. Created models and solids as well as all created objects will be visible and editable 
in tree view. Created object is placed (at least for now) in front of the camera view.

![image](https://github.com/user-attachments/assets/264ad4bf-e50d-42a2-9d8f-e5b25d8d9086)

Clicking the specific object in tree view will open settings for the object

![image](https://github.com/user-attachments/assets/9940a236-be25-4bcb-87de-d460c1ab2c15)

Mesh visibility, script binding and animation viewer are avalible in here.

Placing light and sound objects work the same way. All objects exist in space. Sound will have a "source" at specific coords in space

### Scripting
Scripting is done by compiling your own dll file with minialistic header provided by the engine (right now called EverettScript.h and located in ScriptTest) including glm and interfaces to object types (ISolidSim, ILightSim, ISoundSim, ICameraSim and general IObjectSim)

To bind an object to script dll use object options, select DLL and load. If you need to change code in the dll - unload dll, recompile and load dll again. Previously binded objects and keybinds will be rebinded again automatically

![image](https://github.com/user-attachments/assets/2cdc36cd-f713-459e-97da-9e94952e4eae)

You can also bind your camera object (this binds a main-like function in script dll) and bind your key on the keyboard. In keybind options click the button to detect next key pressed and load DLL.The dll code should use appropriate macro and name you see for the key. The names for keys are simple and self-explanatory - M for M, Space for Space, etc.

![image](https://github.com/user-attachments/assets/ab79b62a-7001-40e7-9d1e-d46f196f0a11)


Provided header includes macros that declare appropriate functions to use:

`ScriptObjectInit(name, objectType)` - Executes once when object is binded. name - name of the object, objectType - type of it's interface (ISolidSim for Solid etc.)

`CameraScriptLoop()` - Executes each frame, used as a "main" for the dll. Each dll can have it's own "main", but does not need to one to work correctly

`ScriptKeybindPressed(keyname)` - Executes each time button is pressed, keyname is the name of key you see after the bind in keybind options

`ScriptKeybindReleased(keyname)` - Executes each time button is released, same as above

Usage example:
```cpp
ScriptObjectInit(stickAnimTest, ISolidSim)
{
	// we get address for the object for our interface,
	// choose animation and play it on object intialization for this script
	interState.animCharInter = &objectstickAnimTest;
	interState.animCharInter->SetModelAnimation(0);
	interState.animCharInter->PlayModelAnimation();
}
```

> The interfaces' methods are simple and intuitive, however one important thing should be pointed out: You are able to execute initilization macro of an object from the interface itself as well as from the engine. Meaning, by calling `ExecuteScriptFunc` (with required dll name) or `ExecuteAllScriptFuncs` it's possible to execute script initialization functions of objects in other script DLLs, allowing for inter-DLL communication directly from script code

### Animations
Animations can be playbacked through the engine or (as previously demonstrated) from scripting. 

![video_2025-05-08_18-21-12](https://github.com/user-attachments/assets/93b8679e-c84e-4252-9eca-4b467f51ec77)

Engine supports unlimited amount of animations to be playbacked at different times for different solids as well. (The internally used shader will be recompiled dynamically see ShaderGenerator)

### World saving
You can save and load your work from Save/Load. Custom esav file will be created at chosen directory. This feature is WIP at the moment

## LambdaGL (LGL)
Is a lightweight wrapper around OpenGL combined with GLAD, GLFW, GLM

API reference

`InitOpenGL` - Initializes OpenGL according to chosen version (tested fully on 3.3)

`TerminateOpenGL` - Terminates OpenGL

`CreateWindow` - Creates render window, returns true on success

`CreateMesh` - Creates mesh based on provided MeshInfo
 
`CreateModel` - Creates model based on provided ModelInfo

`SetDepthTest` - Sets depth test for intance's window, see `DepthTestMode` enum in the header

`GetMaxAmountOfVertexAttr` - Gets amount of avalible vertex attributes that can be used in a shader

`CaptureMouse` - Captures mouse/cursor if `true` is passed, uncaptures on `false`

`SetInteractable` - Binds a key to a predicate (on press and on release). Holdable if `true` - will call the passed on press function on each frame, if `false` - only once until key is released and pressed once more
	
`ConvertKeyTo` - Converts a key id to readable key name, or key name to key id

`SetAssetOnOpenGLFailure` - If `true` will create assertion on failure of OpenGL function. Set to `false` by default

`SetShaderFolder` - Sets current folder with shader files

`RecompileShader` - Forces a shader recompile

`SetCursorPositionCallback` - Sets a function that will be called on each cursor position change

`SetScrollCallback` - Sets a function that will be called on each mouse scroll trigger

`SetKeyPressCallback` - Sets a function that will be called on any key press

`SetShaderUniformValue` - Sends a value to shader through uniform name, can send to specific shader program but sends to currently used by default, supports `int`, `float`, `unsigned int`, `glm::vec` types and `glm::mat` types

Minimalistic usage example:
```cpp
#include <thread>

#include "LGL.h"
#include "LGLStructs.h"

int main()
{
	// Choose your OpenGL version to use
	LGL::InitOpenGL(3, 3);

	// Can have unlimited amout of instances, context management is automatic
	LGL lgl;

	// Initializes GLAD, GLFW as well
	lgl.CreateWindow(800, 600, "MainWindow");

	// Setup shader path, shader names to be used are a part of ModelInfo
	lgl.SetShaderFolder("C:\\MyShaderFolderPath");

	LGLStructs::ModelInfo model;
	// model = LoadModel() fill up yourself or use your model loader, assimp for example;
	// More info in LGLStructs documentation
	lgl.CreateModel("modelName", model);

	// Will be executed only once each time model is rendered
	model.modelBehaviour = [](){};
	// will be executed only once each time specific mesh is rendered, applies for all meshes
	model.generalMeshBehaviour = [](int meshIndex){};
	// will be executed only once each time THIS specific mesh is rendered 
	// overrides generalMeshBehaviour for this one mesh
	model.meshes[0].behaviour = [](int meshIndex){}; 

	// Add your own predicates, will execute on each key press/release depending on "holdable" param
	lgl.SetInteractable(LGL::ConvertKeyTo("w"), true, [](){}, [](){});

	std::thread renderThread([&lgl]() // Must run in a separate thread
	{
		// Will be executed each frame
		lgl.RunRenderingCycle([&lgl]()
		{
			// Predicates will often contain uniform shader value setters, like so
			// supports fundamental types and glm lib types
			lgl.SetShaderUniformValue("valueName", 1.0);
		}
		); 
	}
	);

	// After being done - terminate
	LGL::TerminateOpenGL();

}
```

### Shaders
Shaders in LGL are compiled by name from the folder set with `SetShaderFolder`. 

It's expected to have at least one of the following shader types `.vert` (for vertex shader), `.frag` (for fragment shader) or `.geom` (for geometry shader) for shader program to be created. 

Files have to have identical names for one shader program (`myShader.vert`, `myShader.frag`). 

Shader programs are called the same name as the shader files used (`myShader`). Amount of separate shader programs that you can create is unlimited. Shader program name must be set in `ModelInfo` struct for whole model or `MeshInfo` for specific mesh

### LGLStructs

LGLStructs are a collection of useful prepared structs to use for model loading

`BasicVertex` - Contains `glm::vec3` values for position, normal, texture coords (in practice usually used as `glm::vec2`), tangent and bitangent

`Vertex` - Contains `BasicVertex` + bone information

`Texture` - Contains texture information

`Mesh` - Contains `std::vector`s of `Vertex`, `unsigned int`s for indeces and `Texture`s. 

`MeshInfo` - Contains `Mesh` and additional data about the mesh, for example - used shader program name. By default will use values from `ModelInfo` but can be overriden for specific value

`ModelInfo` - Contains all `MeshInfo` and default values for them. The default values are referenced, therefore changing a value in `ModelInfo` will automatically apply for all `MeshInfo` values

> Custom `stdEx::ValWithBackup` is used from my repo `stdEx` https://github.com/MaxSaganyuk/stdEx
