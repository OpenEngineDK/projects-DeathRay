// main
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// OpenEngine stuff
#include <Meta/Config.h>
#include <Logging/Logger.h>
#include <Logging/StreamLogger.h>
#include <Core/Engine.h>
#include <Renderers/OpenGL/Renderer.h>
#include <Display/Camera.h>
#include <Devices/IKeyboard.h>
#include <Devices/IMouse.h>

// SimpleSetup
#include <Utils/SimpleSetup.h>
#include <Display/SDLEnvironment.h>
#include <Display/Viewport.h>

// *** Death rays ***

// Resources
#include <Resources/ResourceManager.h>
#include <Resources/MHDResource.h>

// Scene and rendering
#include <Renderers/OpenGL/DoseCalcRenderingView.h>
#include <Scene/DoseCalcNode.h>

// Tools
#include <Utils/MouseSelection.h>
#include <Utils/CameraTool.h>
#include <Utils/ToolChain.h>
#include <Utils/WidgetTool.h>
#include <Resources/SDLFont.h>

// name spaces that we will be using.
// this combined with the above imports is almost the same as
// fx. import OpenEngine.Logging.*; in Java.
using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Display;
using namespace OpenEngine::Renderers;
using namespace OpenEngine::Renderers::OpenGL;

/**
 * Main method for the first quarter project of CGD.
 * Corresponds to the
 *   public static void main(String args[])
 * method in Java.
 */
int main(int argc, char** argv) {
    // Create Viewport and renderingview
    IEnvironment* env = new SDLEnvironment(800,600);
    Viewport* vp = new Viewport(env->GetFrame());
    IRenderingView* rv = new DoseCalcRenderingView(*vp);

    // Create simple setup
    SimpleSetup* setup = new SimpleSetup("Death by tray.. I mean Death Ray", vp, env, rv);


    // Setup Loaders
    ResourceManager<ITexture3DResource>::AddPlugin(new MHDResourcePlugin());
    ResourceManager<IFontResource>::AddPlugin(new SDLFontPlugin());
    DirectoryManager::AppendPath("projects/DeathRay/data/");    

    // Setup Camera
    Camera* cam = setup->GetCamera();
    cam->SetPosition(Vector<3, float>(-256.0, 200.0, -256.0));
    cam->LookAt(0.0, 127.0, 0.0);
    
    // Setup the scene
    setup->GetRenderer().SetBackgroundColor(Vector<4, float>(0, 0, 0, 1.0));
    
    ITexture3DResourcePtr mhd = ResourceManager<ITexture3DResource>::Create("20-P.mhd");
    DoseCalcNode* doseNode = new DoseCalcNode(mhd);
    setup->GetRenderer().InitializeEvent().Attach(*doseNode);

    // setup the scene graph
    ISceneNode* root = setup->GetScene();
    root->AddNode(doseNode);
    
    // Setup edit tools
    ToolChain* chain = new ToolChain();
    WidgetTool* wt = new WidgetTool(setup->GetTextureLoader());
    chain->PushBackTool(wt);
    wt->AddWidget(doseNode->GetWidget());

    CameraTool* ct = new CameraTool();
    chain->PushBackTool(ct);    

    MouseSelection* ms = new MouseSelection(env->GetFrame(), setup->GetMouse(), NULL);
    ms->BindTool(vp, chain);
    
    setup->GetKeyboard().KeyEvent().Attach(*ms);
    setup->GetMouse().MouseMovedEvent().Attach(*ms);
    setup->GetMouse().MouseButtonEvent().Attach(*ms);
    setup->GetRenderer().PostProcessEvent().Attach(*ms);


    // Start the engine.
    setup->GetEngine().Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}


