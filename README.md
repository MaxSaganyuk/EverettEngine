<p align="center">
<img width="500" height="250" alt="title" src="docs/title.png" />
</p>

# ProjectEverett

ProjectEverett is a combination of two projects - Everett (the game engine), LGL (the OpenGL wrapper). For now, Everett is documented as a guide (how to use the application) and LGL is documented as an API (how to use in your code with), this might change later.

Document is accurate for 03.05.2026 version of the code

## Everett

Everett is a minimalistic 3D game engine. Right now uses MFC for GUI, and LGL for rendering but since it's separated, can be reworked to use other GUI lib or rendering lib.

Third party includes: Assimp for model loading with animation info, OpenAL for spatial sound, DrWav for Wav file loading, Freetype for font loading, stb_image for image (texture) loading, OpenSSL for (not yet implemented) future encryption for networking. 

Main windows looks like so. Window on the left is a render window, on the right - interactable GUI:

![image](docs/pic1.png)

Features include:

- Object placing (Model loading and solid, light, sound and collider placing in virtual space)

- Scripting in C++ with DLL hot loading

- Unlimited animation playback with adaptive internal shader

- World saving and loading

- Game setup

### Object placing

You can load a model from `obj`, `glb`, `dae`, `fdx` file types and place them as a solid in space. Created models and solids as well as all created objects will be visible and editable in tree view. Created object is placed in front of the camera view and rotated to "look" at the +Z axis by default.

Clicking the specific object in tree view will open settings for the object. Mesh visibility, script binding and animation viewer are avalible in here.

Placing light and sound objects work the same way. All objects exist in space. Sound will have a "source" at specific coords in space

Light, sound and collider objects are represented in space via transparent colored gizmos. Yellow for light, blue for sound

![image](docs/pic2.png)

For colliders there are two colors - green for non colliding and red for colliding (all gifs below have lower FPS than the engine is capable of)

![image](docs/mov1.gif)

### Scripting

Scripting is done by compiling your own dll file with minialistic headers provided by the engine (located in "external" folder) including glm and interfaces to the engine (IEverettEngine) and object types (ISolidSim, ILightSim, ISoundSim, ICameraSim, IColliderSim and general IObjectSim). It is enough to include EverettScript.h to include all you need.

To bind an object to script dll use script dll dialog, select DLL and load. If you need to change code in the dll - unload dll, recompile and load dll again (hot reloading). View IEverettEngine interface header to see what kind of control is possible from scripting.

Provided header includes macros that declare appropriate functions to use:

`ScriptInit()` - Executes on dll load. Intended for initialization.

`MainScriptLoop()` - Executes each frame, used as a "main" for the dll. Each dll can have it's own "main", but does not need to one to work correctly

`ScriptCleanUp()` - Executes on dll unload.

Basic usage example (this is the script that runs moving collider in gif above):

```cpp
IColliderSim* collider{};

ScriptInit()
{
    collider = engine.GetColliderInterface("Box1");

    if (collider)
    {
        engine.AddInteractable(engine.ConvertKeyTo('P'), true, []() { collider->MoveByAxis({-1.0f, 0.0f, 0.0f});  });
        engine.AddInteractable(engine.ConvertKeyTo('I'), true, []() { collider->MoveByAxis({ 1.0f, 0.0f, 0.0f }); });
    }
}

ScriptCleanUp()
{
    collider = nullptr;
}
```

#### Object linking

Object linking is a feature that allows to link calls to objects, this removes necessity to write duplicate code for linked behaviour. Check IObjectSim header for functions which have "Linkable" macro before the name. Even if objects are linked, it's possible to make specific calls not executed "link", check last boolean parameter of the "Linkable" function. 

Box and its collider without object linking:

![image](docs/mov3.gif)

Box and its collider with object linking:

![image](docs/mov4.gif)

#### Binding (key binds and collision binds)

Binding allows custom behaviour (functions) to be executed on specific events. 

Key binds execute when specific key is hit. Collision binds executed on detected collision.

All binds have option called "holdability" - which simply determines whether behavior is executed only once on hit, or on every frame on hit.

All binds have option to add "release" behaviour which is executed only once after hit stops.

For colliders there's two types - "any" and "object binded". "Any" executes behaviour on any collision. Binded collision executed only if collision is detected with a specific collider.

Demonstration of linking and binding (perspective causes illusion of an error):

In this script pressing I and P keys moves the collider, pressing R key rotates the box, which rotates box collider due to linking, and if box collider and (specifically) "free" collider hit - spot light in the scene changes color to blue. When collision stops - back to yellow. 

![image](docs/mov6.gif)

Code that executes this behaviour:

```cpp
ISolidSim* box{};
IColliderSim* boxCollider{};
ILightSim* light{};
IColliderSim* freeCollider{};

ScriptInit()
{
    box = engine.GetSolidInterface("box1");

    if (box)
    {
        engine.AddInteractable(engine.ConvertKeyTo('R'), true, []() { box->Rotate({ 0.0f, 0.05f, 0.0f }); });
    }

    boxCollider = engine.GetColliderInterface("Box");

    if (boxCollider)
    {
        box->LinkObject(*boxCollider);
    }

    light = engine.GetLightInterface("Spot");
    freeCollider = engine.GetColliderInterface("Box1");

    if (freeCollider && light)
    {
        engine.AddInteractable(engine.ConvertKeyTo('P'), true, []() { freeCollider->MoveByAxis({ -1.0f, 0.0f, 0.0f }); });
        engine.AddInteractable(engine.ConvertKeyTo('I'), true, []() { freeCollider->MoveByAxis({  1.0f, 0.0f, 0.0f }); });

        // Here we use "binded object" and negative holdability
        freeCollider->AddCollisionCallback({
            []() { light->GetColorVectorAddr() = { 0.0f, 0.0f, 1.0f }; },
            []() { light->GetColorVectorAddr() = { 1.0f, 1.0f, 0.0f }; },
            boxCollider,
            false
        });
    }
}

ScriptCleanUp()
{
    box = nullptr;
    boxCollider = nullptr;
    freeCollider = nullptr;
    light = nullptr;
}
```

### Animations

Animations can be playbacked through the engine or from scripting. 

![image](docs/mov2.gif)

Engine supports unlimited amount of animations to be playbacked at different times for different solids as well. (The internally used shader will be recompiled dynamically see ShaderGenerator)

### World saving

You can save and load your work from Save/Load. Custom esav file will be created at chosen directory. World save tracks dll hash to make sure that loading script dll was not changed.

### Game setup

It's possible to produce a runnable game via EverettPlayer.exe. 

![image](docs/pic3.png)

After confirming folder and starting world save - required exe, dlls and assets will automatically be copied to chosen folder.

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
