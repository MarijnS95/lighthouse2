/* main.cpp - Copyright 2019 Utrecht University

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "platform.h"
#include "rendersystem.h"

#include <bitset>

#include <bitset>

static RenderAPI* renderer = 0;
static GLTexture* renderTarget = 0;
static Shader* shader = 0;
static uint scrwidth = 0, scrheight = 0, scrspp = 1;
static bool camMoved = false, spaceDown = false, hasFocus = true, running = true, animPaused = false;
static std::bitset<1024> keystates;
static std::bitset<8> mbstates;

// material editing
HostMaterial *currentMaterial = nullptr;
int currentMaterialID = -1;
static CoreStats coreStats;

#include "main_tools.h"

#include <materials/pbrt/api.h>

using namespace pbrt;

//  +-----------------------------------------------------------------------------+
//  |  PrepareScene                                                               |
//  |  Initialize a scene.                                                  LH2'19|
//  +-----------------------------------------------------------------------------+
void PrepareScene()
{
	// initialize scene
#if 0
	renderer->AddScene( "scene.gltf", "data/pica/", mat4::Translate( 0, -10.2f, 0 ) );
	// int meshIdx = renderer->AddMesh( "rungholt.obj", "data/rungholt/", 1.0f );
	// renderer->AddInstance( meshIdx );
	// renderer->AddScene( "CesiumMan.glb", "data/", mat4::Translate( 0, -2, -9 ) );
	// renderer->AddScene( "project_polly.glb", "data/", mat4::Translate( 4.5f, -5.45f, -5.2f ) * mat4::Scale( 2 ) );
	int rootNode = renderer->FindNode( "RootNode (gltf orientation matrix)" );
	renderer->SetNodeTransform( rootNode, mat4::RotateX( -PI / 2 ) );
	int lightMat = renderer->AddMaterial( make_float3( 100, 100, 80 ) );
	int lightQuad = renderer->AddQuad( make_float3( 0, -1, 0 ), make_float3( 0, 26.0f, 0 ), 6.9f, 6.9f, lightMat );
	int lightInst = renderer->AddInstance( lightQuad );

	common::materials::pbrt::Glass glass{};
	glass.R = make_float3( .9f );
	glass.T = make_float3( .9f );
	glass.urough = .001f;
	glass.vrough = .001f;
	glass.eta = 1.5f;
	// glass.remapRoughness = false;

	common::materials::pbrt::Substrate substrate{};
	substrate.Ks = make_float3( 0.04f );
	substrate.urough = 0.001f;
	substrate.vrough = 0.001f;
	substrate.Kd = make_float3( 1.000000, 0.378676, 0.013473 );

	common::materials::pbrt::Mirror mirror{};
	mirror.Kr = make_float3( .9f );

	common::materials::pbrt::Metal metal{};
	metal.eta = make_float3( 1.657460, 0.880369, 0.521229 );
	metal.k = make_float3( 9.223869, 6.269523, 4.837001 );
	metal.urough = .1f;
	metal.vrough = .1f;
	metal.remapRoughness = false;

	common::materials::pbrt::Matte matte{};
	matte.Kd = make_float3( 0.325000, 0.310000, 0.250000 );
	matte.sigma = 90.f;

	common::materials::pbrt::Disney disney{};
	disney.metallic = 1.f;
	disney.color = make_float3( .5f );
	disney.eta = 1.5f;

	common::materials::pbrt::Plastic plastic{};
	plastic.Ks = make_float3( 1, 0, 1 );
	plastic.Kd = make_float3( 0, .25f, 0 );
	plastic.roughness = 0.1f;
	plastic.remapRoughness = false; //true;

	// TODO: Just get a list of materials.
	for ( int i = 0; i < 35; ++i )
	{
		constexpr auto matcount = 5;
		auto& mat = renderer->GetMaterial( i );
		if ( i % matcount == 0 )
			mat = new SimpleHostMaterial<decltype( glass )>( glass );
		else if ( i % matcount == 1 )
			mat = new SimpleHostMaterial<decltype( substrate )>( substrate );
		else if ( i % matcount == 2 )
			mat = new SimpleHostMaterial<decltype( mirror )>( mirror );
		else if ( i % matcount == 3 )
			mat = new SimpleHostMaterial<decltype( metal )>( metal );
		else
			mat = new SimpleHostMaterial<decltype( matte )>( matte );

		// mat = new SimpleHostMaterial<decltype( disney )>( disney );
		mat = new SimpleHostMaterial<decltype( plastic )>( plastic );
	}

	// read persistent material changes
	// renderer->DeserializeMaterials( "data/pica/pica_materials.xml" );

#else

	Options opts;

	pbrtInit( opts, renderer->GetScene() );

	// pbrtParseFile( "data/coffee/scene.pbrt" );
	pbrtParseFile( "data/material-testball/scene.pbrt" );
	// pbrtParseFile( "data/glass-of-water/scene.pbrt" );

	// TODO in end of main!
	// pbrtCleanup();

#endif
}

//  +-----------------------------------------------------------------------------+
//  |  HandleInput                                                                |
//  |  Process user input.                                                  LH2'19|
//  +-----------------------------------------------------------------------------+
bool HandleInput( float frameTime )
{
	// handle keyboard input
	float tspd = (keystates[GLFW_KEY_LEFT_SHIFT] ? 15.0f : 5.0f) * frameTime, rspd = 2.5f * frameTime;
	bool changed = false;
	Camera *camera = renderer->GetCamera();
	if (keystates[GLFW_KEY_A]) { changed = true; camera->TranslateRelative( make_float3( -tspd, 0, 0 ) ); }
	if (keystates[GLFW_KEY_D]) { changed = true; camera->TranslateRelative( make_float3( tspd, 0, 0 ) ); }
	if (keystates[GLFW_KEY_W]) { changed = true; camera->TranslateRelative( make_float3( 0, 0, tspd ) ); }
	if (keystates[GLFW_KEY_S]) { changed = true; camera->TranslateRelative( make_float3( 0, 0, -tspd ) ); }
	if (keystates[GLFW_KEY_R]) { changed = true; camera->TranslateRelative( make_float3( 0, tspd, 0 ) ); }
	if (keystates[GLFW_KEY_F]) { changed = true; camera->TranslateRelative( make_float3( 0, -tspd, 0 ) ); }
	if (keystates[GLFW_KEY_B]) changed = true; // force restart
	if (keystates[GLFW_KEY_UP]) { changed = true; camera->TranslateTarget( make_float3( 0, -rspd, 0 ) ); }
	if (keystates[GLFW_KEY_DOWN]) { changed = true; camera->TranslateTarget( make_float3( 0, rspd, 0 ) ); }
	if (keystates[GLFW_KEY_LEFT]) { changed = true; camera->TranslateTarget( make_float3( -rspd, 0, 0 ) ); }
	if (keystates[GLFW_KEY_RIGHT]) { changed = true; camera->TranslateTarget( make_float3( rspd, 0, 0 ) ); }
	if (!keystates[GLFW_KEY_SPACE]) spaceDown = false; else { if (!spaceDown) animPaused = !animPaused, changed = true; spaceDown = true; }
	// process left button click
	if (mbstates[GLFW_MOUSE_BUTTON_1] && keystates[GLFW_KEY_LEFT_SHIFT])
	{
		int selectedMaterialID = renderer->GetTriangleMaterialID( coreStats.probedInstid, coreStats.probedTriid );
		if (selectedMaterialID != -1)
		{
			auto disneyMaterial = dynamic_cast<HostMaterial*>( renderer->GetMaterial( selectedMaterialID ) );
			if ( disneyMaterial )
			{
				currentMaterial = disneyMaterial;
				currentMaterialID = selectedMaterialID;
				currentMaterial->Changed(); // update checksum so we can track changes
			}
		}
		// camera->focalDistance = coreStats.probedDist;
		changed = true;
	}
	if ( keystates[GLFW_KEY_SPACE] )
	{
		// changed stays false!
		// TODO: This should be debounced...
		animPaused = !animPaused;
	}
	// let the main loop know if the camera should update
	return changed;
}

//  +-----------------------------------------------------------------------------+
//  |  HandleMaterialChange                                                       |
//  |  Update a scene material based on AntTweakBar.                        LH2'19|
//  +-----------------------------------------------------------------------------+
bool HandleMaterialChange()
{
	bool changed = currentMaterial && currentMaterial->Changed();
	if (changed)
		// Checking ->Changed() does not only compare CRC, but also update it.
		// Meaning that from that point on ->Changed() returns false. Mark
		// it as dirty again:
		currentMaterial->MarkAsDirty();
	return changed;
}

//  +-----------------------------------------------------------------------------+
//  |  main                                                                       |
//  |  Application entry point.                                             LH2'19|
//  +-----------------------------------------------------------------------------+
int main()
{
	// initialize OpenGL and ImGui
	InitGLFW();
	InitImGui();

	// initialize renderer: pick one
	// renderer = RenderAPI::CreateRenderAPI( "RenderCore_Optix7Filter" );			// OPTIX7 core, with filtering (static scenes only for now)
	// renderer = RenderAPI::CreateRenderAPI( "RenderCore_Optix7" );			// OPTIX7 core, best for RTX devices
	// renderer = RenderAPI::CreateRenderAPI( "RenderCore_Vulkan_RT" );			// Meir's Vulkan / RTX core
	renderer = RenderAPI::CreateRenderAPI( "RenderCore_OptixPrime_B" );		// OPTIX PRIME, best for pre-RTX CUDA devices
	// renderer = RenderAPI::CreateRenderAPI( "RenderCore_PrimeRef" );			// REFERENCE, for image validation
	// renderer = RenderAPI::CreateRenderAPI( "RenderCore_SoftRasterizer" );	// RASTERIZER, your only option if not on NVidia

	renderer->DeserializeCamera( "camera.xml" );
	// initialize scene
	PrepareScene();
	// set initial window size
	ReshapeWindowCallback( 0, SCRWIDTH, SCRHEIGHT );
	// enter main loop
	Timer timer;
	timer.reset();
	float deltaTime = 0;
	while (!glfwWindowShouldClose( window ))
	{
		// detect camera changes
		camMoved = false;
		deltaTime = timer.elapsed();
		if (HandleInput( deltaTime )) camMoved = true;
		// handle material changes
		if (HandleMaterialChange()) camMoved = true;
		// poll events, may affect probepos so needs to happen between HandleInput and Render
		glfwPollEvents();
		// update animations
		if (!animPaused) for (int i = 0; i < renderer->AnimationCount(); i++)
		{
			renderer->UpdateAnimation( i, deltaTime );
			camMoved = true; // will remain false if scene has no animations
		}
		renderer->SynchronizeSceneData();
		// render
		timer.reset();
		renderer->Render( camMoved ? Restart : Converge );
		// postprocess
		shader->Bind();
		shader->SetInputTexture( 0, "color", renderTarget );
		shader->SetInputMatrix( "view", mat4::Identity() );
		DrawQuad();
		shader->Unbind();
		// gui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin( "Render statistics", 0 );
		coreStats = renderer->GetCoreStats();
		SystemStats systemStats = renderer->GetSystemStats();
		ImGui::Text( "Frame time:   %6.2fms", coreStats.renderTime * 1000 );
		ImGui::Text( "Scene update: %6.2fms", systemStats.sceneUpdateTime * 1000 );
		ImGui::Text( "Primary rays: %6.2fms", coreStats.traceTime0 * 1000 );
		ImGui::Text( "Secondary:    %6.2fms", coreStats.traceTime1 * 1000 );
		ImGui::Text( "Deep rays:    %6.2fms", coreStats.traceTimeX * 1000 );
		ImGui::Text( "Shadow rays:  %6.2fms", coreStats.shadowTraceTime * 1000 );
		ImGui::Text( "Shading time: %6.2fms", coreStats.shadeTime * 1000 );
		ImGui::Text( "Filter time:  %6.2fms", coreStats.filterTime * 1000 );
		ImGui::Text( "# primary:    %6ik (%6.1fM/s)", coreStats.primaryRayCount / 1000, coreStats.primaryRayCount / (max( 1.0f, coreStats.traceTime0 * 1000000 )) );
		ImGui::Text( "# secondary:  %6ik (%6.1fM/s)", coreStats.bounce1RayCount / 1000, coreStats.bounce1RayCount / (max( 1.0f, coreStats.traceTime1 * 1000000 )) );
		ImGui::Text( "# deep rays:  %6ik (%6.1fM/s)", coreStats.deepRayCount / 1000, coreStats.deepRayCount / (max( 1.0f, coreStats.traceTimeX * 1000000 )) );
		ImGui::Text( "# shadw rays: %6ik (%6.1fM/s)", coreStats.totalShadowRays / 1000, coreStats.totalShadowRays / (max( 1.0f, coreStats.shadowTraceTime * 1000000 )) );
		ImGui::End();
		if (currentMaterial) {
			ImGui::Begin( "Material parameters", 0 );
			ImGui::Text( "name:    %s", currentMaterial->name.c_str() );
			ImGui::ColorEdit3( "color", (float*)&currentMaterial->color );
			ImGui::ColorEdit3( "absorption", (float*)&currentMaterial->absorption );
			ImGui::SliderFloat( "metallic", &currentMaterial->metallic, 0, 1 );
			ImGui::SliderFloat( "subsurface", &currentMaterial->subsurface, 0, 1 );
			ImGui::SliderFloat( "specular", &currentMaterial->specular, 0, 1 );
			ImGui::SliderFloat( "roughness", &currentMaterial->roughness, 0, 1 );
			ImGui::SliderFloat( "specularTint", &currentMaterial->specularTint, 0, 1 );
			ImGui::SliderFloat( "anisotropic", &currentMaterial->anisotropic, 0, 1 );
			ImGui::SliderFloat( "sheen", &currentMaterial->sheen, 0, 1 );
			ImGui::SliderFloat( "sheenTint", &currentMaterial->sheenTint, 0, 1 );
			ImGui::SliderFloat( "clearcoat", &currentMaterial->clearcoat, 0, 1 );
			ImGui::SliderFloat( "clearcoatGloss", &currentMaterial->clearcoatGloss, 0, 1 );
			ImGui::SliderFloat( "transmission", &currentMaterial->transmission, 0, 1 );
			ImGui::SliderFloat( "eta", &currentMaterial->eta, 1, 3 );
			ImGui::End();
		}
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
		// finalize
		glfwSwapBuffers( window );
		// terminate
		if (!running) break;
	}
	// clean up
	renderer->SerializeCamera( "camera.xml" );
	renderer->Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow( window );
	glfwTerminate();
	return 0;
}

// EOF
