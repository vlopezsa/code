#include "Deferred.h"

const std::string Deferred::skDefaultModel = "Arcade/Arcade.fbx";

Deferred::~Deferred()
{
}

CameraController& Deferred::getActiveCameraController()
{
	return mFirstPersonCameraController;
}

void Deferred::loadModelFromFile(const std::string& filename, Fbo* pTargetFbo)
{
	Model::LoadFlags flags = Model::LoadFlags::None;
	if (mGenerateTangentSpace == false)
	{
		flags |= Model::LoadFlags::DontGenerateTangentSpace;
	}
	auto fboFormat = pTargetFbo->getColorTexture(0)->getFormat();
	flags |= isSrgbFormat(fboFormat) ? Model::LoadFlags::None : Model::LoadFlags::AssumeLinearSpaceTextures;

	mpModel = Model::createFromFile(filename.c_str(), flags);

	if (mpModel == nullptr)
	{
		msgBox("Could not load model");
		return;
	}
	resetCamera();
}

void Deferred::loadModel(Fbo* pTargetFbo)
{
	std::string filename;
	if (openFileDialog("Supported Formats\0*.obj;*.bin;*.dae;*.x;*.md5mesh\0\0", filename))
	{
		loadModelFromFile(filename, pTargetFbo);
	}
}

void Deferred::onGuiRender(SampleCallbacks* pSample, Gui* pGui)
{
	// Load model group
	if (pGui->addButton("Load Model"))
	{
		loadModel(pSample->getCurrentFbo().get());
	}

	if (pGui->beginGroup("Load Options"))
	{
		pGui->addCheckBox("Generate Tangent Space", mGenerateTangentSpace);
		pGui->endGroup();
	}

	Gui::DropdownList debugModeList;
	debugModeList.push_back({ 0, "Disabled" });
	debugModeList.push_back({ 1, "Positions" });
	debugModeList.push_back({ 2, "Normals" });
	debugModeList.push_back({ 3, "Albedo" });
	debugModeList.push_back({ 4, "Illumination" });
	pGui->addDropdown("Debug mode", debugModeList, (uint32_t&)mDebugMode);

	Gui::DropdownList cullList;
	cullList.push_back({ 0, "No Culling" });
	cullList.push_back({ 1, "Backface Culling" });
	cullList.push_back({ 2, "Frontface Culling" });
	pGui->addDropdown("Cull Mode", cullList, (uint32_t&)mCullMode);

	if (pGui->beginGroup("Lights"))
	{
		pGui->addRgbColor("Ambient intensity", mAmbientIntensity);
		if (pGui->beginGroup("Directional Light"))
		{
			mpDirLight->renderUI(pGui);
			pGui->endGroup();
		}
		pGui->endGroup();
	}

	if (mpModel)
	{
		renderModelUiElements(pGui);
	}
}

void Deferred::renderModelUiElements(Gui* pGui)
{
	bool bAnim = mpModel->hasAnimations();
	static const char* animateStr = "Animate";
	static const char* activeAnimStr = "Active Animation";

	if (bAnim)
	{
		mActiveAnimationID = sBindPoseAnimationID;

		pGui->addCheckBox(animateStr, mAnimate);
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
		if (pGui->addDropdown(activeAnimStr, list, mActiveAnimationID))
		{
			mpModel->setActiveAnimation(mActiveAnimationID);
		}
	}
	if (pGui->beginGroup("Depth Range"))
	{
		const float minDepth = mpModel->getRadius() * 1 / 1000;
		pGui->addFloatVar("Near Plane", mNearZ, minDepth, mpModel->getRadius() * 15, minDepth * 5);
		pGui->addFloatVar("Far Plane", mFarZ, minDepth, mpModel->getRadius() * 15, minDepth * 5);
		pGui->endGroup();
	}
}

void Deferred::onLoad(SampleCallbacks* pSample, RenderContext::SharedPtr pRenderContext)
{
	mpCamera = Camera::create();

	mpDeferredPassProgram = GraphicsProgram::createFromFile("", appendShaderExtension("DeferredPass.ps"));

	mpLightingPass = FullScreenPass::create(appendShaderExtension("LightingPass.ps"));

	// create rasterizer state
	RasterizerState::Desc rsDesc;
	mpCullRastState[0] = RasterizerState::create(rsDesc);
	rsDesc.setCullMode(RasterizerState::CullMode::Back);
	mpCullRastState[1] = RasterizerState::create(rsDesc);
	rsDesc.setCullMode(RasterizerState::CullMode::Front);
	mpCullRastState[2] = RasterizerState::create(rsDesc);

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

	// Load default model
	loadModelFromFile(skDefaultModel, pSample->getCurrentFbo().get());
}

void Deferred::onFrameRender(SampleCallbacks* pSample, RenderContext::SharedPtr pRenderContext, Fbo::SharedPtr pTargetFbo)
{
	GraphicsState* pState = pRenderContext->getGraphicsState().get();

	const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);

	// G-Buffer pass
	if (mpModel)
	{
		pRenderContext->clearFbo(mpGBufferFbo.get(), glm::vec4(0), 1.0f, 0, FboAttachmentType::Color | FboAttachmentType::Depth);
		pState->setFbo(mpGBufferFbo);

		mpCamera->setDepthRange(mNearZ, mFarZ);
		CameraController& ActiveController = getActiveCameraController();
		ActiveController.update();

		// Animate
		if (mAnimate)
		{
			PROFILE(Animate);
			mpModel->animate(pSample->getCurrentTime());
		}

		// Set render state
		pState->setRasterizerState(mpCullRastState[mCullMode]);
		pState->setDepthStencilState(mpDepthTestDS);

		// Render model
		mpModel->bindSamplerToMaterials(mpLinearSampler);
		pRenderContext->setGraphicsVars(mpDeferredVars);
		pState->setProgram(mpDeferredPassProgram);
		ModelRenderer::render(pRenderContext.get(), mpModel, mpCamera.get());
	}

	// Lighting pass (fullscreen quad)
	{
		pState->setFbo(pTargetFbo);
		pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::Color);

		// Reset render state
		pState->setRasterizerState(mpCullRastState[0]);
		pState->setBlendState(mpOpaqueBS);
		pState->setDepthStencilState(mpNoDepthDS);

		// Set lighting params
		ConstantBuffer::SharedPtr pLightCB = mpLightingVars["PerImageCB"];
		pLightCB["gAmbient"] = mAmbientIntensity;
		mpDirLight->setIntoProgramVars(mpLightingVars.get(), pLightCB.get(), "gDirLight");
		// Debug mode
		pLightCB->setVariable("gDebugMode", (uint32_t)mDebugMode);

		// Set GBuffer as input
		mpLightingVars->setTexture("gGBuf0", mpGBufferFbo->getColorTexture(0));
		mpLightingVars->setTexture("gGBuf1", mpGBufferFbo->getColorTexture(1));
		mpLightingVars->setTexture("gGBuf2", mpGBufferFbo->getColorTexture(2));


		// Kick it off
		pRenderContext->setGraphicsVars(mpLightingVars);
		mpLightingPass->execute(pRenderContext.get());
	}
}

void Deferred::onShutdown(SampleCallbacks* pSample)
{
	mpModel.reset();
}

bool Deferred::onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent)
{
	bool bHandled = getActiveCameraController().onKeyEvent(keyEvent);
	if (bHandled == false)
	{
		if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
		{
			switch (keyEvent.key)
			{
			case KeyboardEvent::Key::R:
				resetCamera();
				break;
			default:
				bHandled = false;
			}
		}
	}
	return bHandled;
}

bool Deferred::onMouseEvent(SampleCallbacks* pSample, const MouseEvent& mouseEvent)
{
	return getActiveCameraController().onMouseEvent(mouseEvent);
}

void Deferred::onResizeSwapChain(SampleCallbacks* pSample, uint32_t width, uint32_t height)
{
	mpCamera->setFocalLength(21.0f);
	mAspectRatio = (float(width) / float(height));
	mpCamera->setAspectRatio(mAspectRatio);
	// create G-Buffer
	const glm::vec4 clearColor(0.f, 0.f, 0.f, 0.f);
	Fbo::Desc fboDesc;
	fboDesc.setColorTarget(0, Falcor::ResourceFormat::RGBA16Float).setColorTarget(1, Falcor::ResourceFormat::RGBA16Float).setColorTarget(2, Falcor::ResourceFormat::RGBA16Float).setDepthStencilTarget(Falcor::ResourceFormat::D32Float);
	mpGBufferFbo = FboHelper::create2D(width, height, fboDesc);
}

void Deferred::resetCamera()
{
	if (mpModel)
	{
		// update the camera position
		float radius = mpModel->getRadius();
		const glm::vec3& modelCenter = mpModel->getCenter();
		glm::vec3 camPos = modelCenter;
		camPos.z += radius * 4;

		mpCamera->setPosition(camPos);
		mpCamera->setTarget(modelCenter);
		mpCamera->setUpVector(glm::vec3(0, 1, 0));

		// Update the controllers
		mFirstPersonCameraController.setCameraSpeed(radius*0.25f);
		mNearZ = std::max(0.1f, mpModel->getRadius() / 750.0f);
		mFarZ = radius * 10;
	}
}

void Deferred::onInitializeTesting(SampleCallbacks* pSample)
{
	auto argList = pSample->getArgList();
	std::vector<ArgList::Arg> modeFrames = argList.getValues("incrementDebugMode");
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

void Deferred::onEndTestFrame(SampleCallbacks* pSample, SampleTest* pSampleTest)
{
	uint32_t frameId = pSample->getFrameID();
	if (mChangeModeIt != mChangeModeFrames.end() && frameId >= *mChangeModeIt)
	{
		++mChangeModeIt;
		uint32_t* pMode = (uint32_t*)&mDebugMode;
		*pMode = min(*pMode + 1, (uint32_t)ShowLighting);
	}
}

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#else
int main(int argc, char** argv)
#endif
{
	Deferred::UniquePtr pRenderer = std::make_unique<Deferred>();
	SampleConfig config;
	config.windowDesc.width = 1280;
	config.windowDesc.height = 720;
	config.windowDesc.resizableWindow = true;
	config.windowDesc.title = "Simple Deferred";
#ifdef _WIN32
	Sample::run(config, pRenderer);
#else
	config.argc = (uint32_t)argc;
	config.argv = argv;
	Sample::run(config, pRenderer);
#endif
	return 0;
}
