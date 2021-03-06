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
#include <Display/ViewingVolume.h>

// *** Death rays ***

// Resources
#include <Resources/ResourceManager.h>
#include <Resources/TemplatedMHDResource.h>

// Scene and rendering
#include <Renderers/OpenGL/DoseCalcRenderingView.h>
#include <Renderers/OpenGL/RayCastRenderingView.h>
#include <Scene/DoseCalcNode.h>
#include <Scene/BeamNode.h>
#include <Scene/TransformationNode.h>

// Tools
#include <Utils/SelectionSet.h>
#include <Utils/MouseSelection.h>
#include <Utils/CameraTool.h>
#include <Utils/TransformationTool.h>
#include <Utils/SelectionTool.h>
#include <Utils/ToolChain.h>
#include <Utils/WidgetTool.h>
#include <Resources/SDLFont.h>
#include <Utils/GLSceneSelection.h>
#include <Renderers/OpenGL/DoseCalcSelectionRenderer.h>
#include <Utils/DoseTrigger.h>
#include <Meta/CUDA.h>


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

    float scale = 0.6;
    int width = 1600 * scale;
    int height = 800 * scale;

    // Init CUDA
    INITIALIZE_CUDA();
    
    //add frustrum cameras
    float dist = 1000;

    // left
    Camera* cam_l = new Camera(*(new ViewingVolume()));
    cam_l->SetPosition(Vector<3, float>(0.0, 0.0, dist));
    cam_l->LookAt(0.0, 0.0, 0.0);
    Viewport* vp_l = new Viewport(0,0, width/2,height);
    vp_l->SetViewingVolume(cam_l);
    // OpenGL::RenderingView* rv_l = new OpenGL::RenderingView(*vp_l);
    IRenderingView* rv_l = new DoseCalcRenderingView(*vp_l);

    // right
    // Camera* cam_r = new Camera(*(new ViewingVolume()));
    // cam_r->SetPosition(Vector<3,float>(0,0,dist));
    // cam_r->LookAt(0,0,0);
    Viewport* vp_r = new Viewport(width/2, 0, width,height);
    vp_r->SetViewingVolume(cam_l);
    RayCastRenderingView* rv_r = new RayCastRenderingView(*vp_r);

    // Create Viewport and renderingview
    IEnvironment* env = new SDLEnvironment(width,height);
    // Viewport* vp = new Viewport(env->GetFrame());

    // Create simple setup
    SimpleSetup* setup = new SimpleSetup("Death by tray.. I mean Death Ray", vp_l, env, rv_l);
    logger.info << "frame: " << width << "x" << height << logger.end;
    //print CUDA
    logger.info << PRINT_CUDA_DEVICE_INFO() << logger.end;
    
    setup->GetRenderer().ProcessEvent().Attach(*rv_r);
    setup->SetCamera(*cam_l);
    setup->GetCamera()->SetPosition(Vector<3, float>(0.0, 0.0, dist));
    setup->GetCamera()->LookAt(0.0, 0.0, 0.0);

    // Setup Loaders
    ResourceManager<MHD(float) >::AddPlugin(new TemplatedMHDResourcePlugin<float>());
    ResourceManager<IFontResource>::AddPlugin(new SDLFontPlugin());
    DirectoryManager::AppendPath("projects/DeathRay/data/");

    // Setup the scene
    setup->GetRenderer().SetBackgroundColor(Vector<4, float>(0, 0, 0, 1.0));

    MHDPtr(float) mhd = ResourceManager<MHD(float) >::Create("20-P.mhd");
    DoseCalcNode* doseNode = new DoseCalcNode(mhd);

    DoseTrigger* dh = new DoseTrigger(doseNode, vp_l); 
    doseNode->AddNode(dh->GetPivotNode());

    setup->GetRenderer().InitializeEvent().Attach(*doseNode);

    // setup the scene graph
    ISceneNode* root = setup->GetScene();
    root->AddNode(doseNode);

    // Setup edit tools
    ToolChain* chain = new ToolChain();

    SelectionSet<ISceneNode>* ss = new SelectionSet<ISceneNode>();
    CameraTool* ct = new CameraTool();
    chain->PushBackTool(ct);

    WidgetTool* wt1 = new WidgetTool(setup->GetTextureLoader());
    chain->PushBackTool(wt1);
    
    Collection* widget = new Collection();
    Collection* w = new DoseCalcNodeWidget(doseNode);
    w->SetFixed(true);
    w->SetBackground(false);
    w->SetPadding(Vector<4,int>(0));
    widget->AddWidget(w);
    w = new DoseTriggerWidget(dh);
    w->SetFixed(true);
    w->SetBackground(false);
    w->SetPadding(Vector<4,int>(0));
    widget->AddWidget(w);
    widget->SetPosition(Vector<2,int>(20,20));
    wt1->AddWidget(widget);

    TransformationTool* tt = new TransformationTool(setup->GetTextureLoader());
    ss->ChangedEvent().Attach(*tt);
    chain->PushBackTool(tt);


    WidgetTool* wt2 = new WidgetTool(setup->GetTextureLoader());
    w = new RayCastRenderingViewWidget(rv_r);
    w->SetPosition(Vector<2,int>(20,20));
    w->SetFixed(false);
    w->SetBackground(true);
    wt2->AddWidget(w);

    SelectionTool* st = new SelectionTool(*ss);
    chain->PushBackTool(st);
    MouseSelection* ms =
        new MouseSelection(env->GetFrame(),
                           setup->GetMouse(),
                           doseNode,
                           new GLSceneSelection(env->GetFrame(), new DoseCalcSelectionRenderer()));

    ms->BindTool(vp_l, chain);
    ms->BindTool(vp_r, wt2);

    setup->GetKeyboard().KeyEvent().Attach(*ms);
    setup->GetMouse().MouseMovedEvent().Attach(*ms);
    setup->GetMouse().MouseButtonEvent().Attach(*ms);
    setup->GetRenderer().PostProcessEvent().Attach(*ms);

    // FPS
    setup->ShowFPS();

    // Start the engine.
    setup->GetEngine().Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}


