#include "Deferred.h"

Deferred::~Deferred()
{
}

void Deferred::reset()
{
	mpRenderer.reset();
	mpScene.reset();
	mpModel.reset();

	mpRenderer = nullptr;
	mpScene = nullptr;
	mpModel = nullptr;
}

CameraController& Deferred::getActiveCameraController()
{
	return mFirstPersonCameraController;
}

void Deferred::loadModelFromFile(const std::string& filename)
{
	Mesh::resetGlobalIdCounter();
	reset();

	Model::LoadFlags flags = Model::LoadFlags::None;
	if (mGenerateTangentSpace == false)
	{
		flags |= Model::LoadFlags::DontGenerateTangentSpace;
	}
	auto fboFormat = mpDefaultFBO->getColorTexture(0)->getFormat();
	flags |= isSrgbFormat(fboFormat) ? Model::LoadFlags::None : Model::LoadFlags::AssumeLinearSpaceTextures;

	mpModel = Model::createFromFile(filename.c_str(), flags);

	if (mpModel == nullptr)
	{
		msgBox("Could not load model");
		return;
	}
	
	mpScene = Scene::create();

	mpScene->addModelInstance(mpModel, "instance");

	initScene();
}

void Deferred::loadModel()
{
	std::string filename;
	if (openFileDialog("Supported Formats\0*.obj;*.bin;*.dae;*.x;*.md5mesh\0\0", filename))
	{
		loadModelFromFile(filename);
	}
}


void Deferred::loadScene()
{
	std::string filename;
	if (openFileDialog(Scene::kFileFormatString, filename))
	{
		reset();

		mpScene = Scene::loadFromFile(filename);

		initScene();
	}
}

void Deferred::initScene()
{
	if (mpScene->getCameraCount() == 0)
	{
		// Place the camera above the center, looking slightly downwards
		const Model* pModel = mpScene->getModel(0).get();

		vec3 position = pModel->getCenter();	
		float radius = pModel->getRadius();
		position.y += 0.1f * radius;
		mpScene->setCameraSpeed(radius * 0.03f);

		mpCamera->setPosition(position);
		mpCamera->setTarget(position + vec3(0, -0.3f, -radius));
		mpCamera->setDepthRange(0.1f, radius * 10);

		mpScene->addCamera(mpCamera);
	}

	if (mpScene->getLightCount() == 0)
	{
		// Create a directional light
		DirectionalLight::SharedPtr pDirLight = DirectionalLight::create();
		pDirLight->setWorldDirection(vec3(-0.189f, -0.861f, -0.471f));
		pDirLight->setIntensity(vec3(1, 1, 0.985f) * 10.0f);
		pDirLight->setName("DirLight");
		mpScene->addLight(pDirLight);
		mpScene->setAmbientIntensity(vec3(0.1f));
	}

	mpRenderer = SceneRenderer::create(mpScene);

	mpRenderer->setCameraControllerType(SceneRenderer::CameraControllerType::FirstPerson);

	mCurrentTime = 0;
}

void Deferred::onGuiRender()
{
	// Load model group
	if (mpGui->addButton("Load Model"))
	{
		loadModel();
	}

	if (mpGui->addButton("Load Scene"))
	{
		loadScene();
	}

	if (mpGui->beginGroup("Load Options"))
	{
		mpGui->addCheckBox("Generate Tangent Space", mGenerateTangentSpace);
		mpGui->endGroup();
	}

	Gui::DropdownList debugModeList;
	debugModeList.push_back({ 0, "Disabled" });
	debugModeList.push_back({ 1, "Positions" });
	debugModeList.push_back({ 2, "Normals" });
	debugModeList.push_back({ 3, "Albedo" });
	debugModeList.push_back({ 4, "Illumination" });
	mpGui->addDropdown("Debug mode", debugModeList, (uint32_t&)mDebugMode);

	if (mpGui->beginGroup("Lights"))
	{
		mpGui->addRgbColor("Ambient intensity", mAmbientIntensity);
		if (mpGui->beginGroup("Directional Light"))
		{
			mpDirLight->renderUI(mpGui.get());
			mpGui->endGroup();
		}
		mpGui->endGroup();
	}

	if (mpModel)
	{
		renderModelUiElements();
	}
}

void Deferred::renderModelUiElements()
{
	bool bAnim = mpModel->hasAnimations();
	static const char* animateStr = "Animate";
	static const char* activeAnimStr = "Active Animation";

	if (bAnim)
	{
		mActiveAnimationID = sBindPoseAnimationID;

		mpGui->addCheckBox(animateStr, mAnimate);
		Gui::DropdownList list;
		list.resize(mpModel->getAnimationsCount() + 1);
		list[0].label = "Bind Pose";
		list[0].value = sBindPoseAnimationID;

		for (uint32_t i = 0; i < mpModel->getAnimationsCount(); i++)
		{
			list[i + 1].value = i;
			list[i + 1].label = mpModel->getAnimationName(i);
			if (list[i + 1].label.size() == 0)
			{
				list[i + 1].label = std::to_string(i);
			}
		}
		if (mpGui->addDropdown(activeAnimStr, list, mActiveAnimationID))
		{
			mpModel->setActiveAnimation(mActiveAnimationID);
		}
	}
	if (mpGui->beginGroup("Depth Range"))
	{
		const float minDepth = mpModel->getRadius() * 1 / 1000;
		mpGui->addFloatVar("Near Plane", mNearZ, minDepth, mpModel->getRadius() * 15, minDepth * 5);
		mpGui->addFloatVar("Far Plane", mFarZ, minDepth, mpModel->getRadius() * 15, minDepth * 5);
		mpGui->endGroup();
	}
}

void Deferred::onLoad()
{
	mpCamera = Camera::create();

	mpDeferredPassProgram = GraphicsProgram::createFromFile("", appendShaderExtension("DeferredPass.ps"));

	mpLightingPass = FullScreenPass::create(appendShaderExtension("LightingPass.ps"));

	// create rasterizer state
	RasterizerState::Desc rsDesc;
	rsDesc.setCullMode(RasterizerState::CullMode::Back);
	mpCullRastState = RasterizerState::create(rsDesc);

	// Depth test
	DepthStencilState::Desc dsDesc;
	dsDesc.setDepthTest(false);
	mpNoDepthDS = DepthStencilState::create(dsDesc);
	dsDesc.setDepthTest(true);
	mpDepthTestDS = DepthStencilState::create(dsDesc);

	// Blend state
	BlendState::Desc blendDesc;
	mpOpaqueBS = BlendState::create(blendDesc);

	mFirstPersonCameraController.attachCamera(mpCamera);

	Sampler::Desc samplerDesc;
	samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear).setMaxAnisotropy(8);
	mpLinearSampler = Sampler::create(samplerDesc);

	mpDirLight = DirectionalLight::create();
	mpDirLight->setWorldDirection(glm::vec3(-0.5f, -0.2f, -1.0f));

	mpDeferredVars = GraphicsVars::create(mpDeferredPassProgram->getActiveVersion()->getReflector());
	mpLightingVars = GraphicsVars::create(mpLightingPass->getProgram()->getActiveVersion()->getReflector());

	initializeTesting();
}

void Deferred::onFrameRender()
{
	beginTestFrame();

	GraphicsState* pState = mpRenderContext->getGraphicsState().get();

	const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);

	// G-Buffer pass
	if (mpScene)
	{
		mpRenderContext->clearFbo(mpGBufferFbo.get(), glm::vec4(0), 1.0f, 0, FboAttachmentType::Color | FboAttachmentType::Depth);
		
		pState->setFbo(mpGBufferFbo);

		mpRenderer->update(mCurrentTime);

		// Animate
		if (mAnimate)
		{
			PROFILE(Animate);
			mpModel->animate(mCurrentTime);
		}

		// Set render state
		pState->setRasterizerState(mpCullRastState);
		pState->setDepthStencilState(mpDepthTestDS);

		// Render model
		mpScene->bindSamplerToMaterials(mpLinearSampler);
		mpRenderContext->setGraphicsVars(mpDeferredVars);
		pState->setProgram(mpDeferredPassProgram);
	
		mpRenderer->renderScene(mpRenderContext.get());
	}

	// Lighting pass (fullscreen quad)
	{
		pState->setFbo(mpDefaultFBO);
		mpRenderContext->clearFbo(mpDefaultFBO.get(), clearColor, 1.0f, 0, FboAttachmentType::Color);

		// Reset render state
		pState->setRasterizerState(mpCullRastState);
		pState->setBlendState(mpOpaqueBS);
		pState->setDepthStencilState(mpNoDepthDS);

		// Set lighting params
		ConstantBuffer::SharedPtr pLightCB = mpLightingVars["PerImageCB"];
		pLightCB["gAmbient"] = mAmbientIntensity;
		mpDirLight->setIntoConstantBuffer(pLightCB.get(), "gDirLight");

		// Debug mode
		pLightCB->setVariable("gDebugMode", (uint32_t)mDebugMode);

		// Set GBuffer as input
		mpLightingVars->setTexture("gGBuf0", mpGBufferFbo->getColorTexture(0));
		mpLightingVars->setTexture("gGBuf1", mpGBufferFbo->getColorTexture(1));
		mpLightingVars->setTexture("gGBuf2", mpGBufferFbo->getColorTexture(2));


		// Kick it off
		mpRenderContext->setGraphicsVars(mpLightingVars);
		mpLightingPass->execute(mpRenderContext.get());
	}

	endTestFrame();
}

void Deferred::onShutdown()
{
	reset();
}

bool Deferred::onKeyEvent(const KeyboardEvent& keyEvent)
{
	return mpRenderer ? mpRenderer->onKeyEvent(keyEvent) : false;
}

bool Deferred::onMouseEvent(const MouseEvent& mouseEvent)
{
	return mpRenderer ? mpRenderer->onMouseEvent(mouseEvent) : true;
}

void Deferred::onResizeSwapChain()
{
	uint32_t width = mpDefaultFBO->getWidth();
	uint32_t height = mpDefaultFBO->getHeight();

	mpCamera->setFocalLength(21.0f);
	mAspectRatio = (float(width) / float(height));
	mpCamera->setAspectRatio(mAspectRatio);
	// create G-Buffer
	const glm::vec4 clearColor(0.f, 0.f, 0.f, 0.f);
	Fbo::Desc fboDesc;
	fboDesc.setColorTarget(0, Falcor::ResourceFormat::RGBA16Float).setColorTarget(1, Falcor::ResourceFormat::RGBA16Float).setColorTarget(2, Falcor::ResourceFormat::RGBA16Float).setDepthStencilTarget(Falcor::ResourceFormat::D32Float);
	mpGBufferFbo = FboHelper::create2D(width, height, fboDesc);
}

void Deferred::onInitializeTesting()
{
	std::vector<ArgList::Arg> modeFrames = mArgList.getValues("incrementDebugMode");
	if (!modeFrames.empty())
	{
		mChangeModeFrames.resize(modeFrames.size());
		for (uint32_t i = 0; i < modeFrames.size(); ++i)
		{
			mChangeModeFrames[i] = modeFrames[i].asUint();
		}
	}

	mChangeModeIt = mChangeModeFrames.begin();
}

void Deferred::onEndTestFrame()
{
	uint32_t frameId = frameRate().getFrameCount();
	if (mChangeModeIt != mChangeModeFrames.end() && frameId >= *mChangeModeIt)
	{
		++mChangeModeIt;
		uint32_t* pMode = (uint32_t*)&mDebugMode;
		*pMode = min(*pMode + 1, (uint32_t)ShowLighting);
	}
}
