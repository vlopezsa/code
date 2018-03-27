#pragma once
#include "Falcor.h"
#include "SampleTest.h"

using namespace Falcor;

class Deferred : public Renderer
{
public:
	~Deferred();

	void onLoad(SampleCallbacks* pSample, RenderContext::SharedPtr pRenderContext) override;
	void onFrameRender(SampleCallbacks* pSample, RenderContext::SharedPtr pRenderContext, Fbo::SharedPtr pTargetFbo) override;
	void onShutdown(SampleCallbacks* pSample) override;
	void onResizeSwapChain(SampleCallbacks* pSample, uint32_t width, uint32_t height) override;
	bool onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent) override;
	bool onMouseEvent(SampleCallbacks* pSample, const MouseEvent& mouseEvent) override;
	void onGuiRender(SampleCallbacks* pSample, Gui* pGui) override;
private:
	void loadModel(Fbo* pTargetFbo);
	void loadModelFromFile(const std::string& filename, Fbo* pTargetFbo);
	void resetCamera();
	void renderModelUiElements(Gui* pGui);

	Model::SharedPtr mpModel = nullptr;
	FirstPersonCameraController mFirstPersonCameraController;
	Sampler::SharedPtr mpLinearSampler;

	GraphicsProgram::SharedPtr mpDeferredPassProgram;
	GraphicsVars::SharedPtr mpDeferredVars;

	GraphicsVars::SharedPtr mpLightingVars;
	FullScreenPass::UniquePtr mpLightingPass;

	float mAspectRatio = 0;

	CameraController& getActiveCameraController();

	Camera::SharedPtr mpCamera;

	bool mAnimate = false;
	bool mGenerateTangentSpace = true;
	glm::vec3 mAmbientIntensity = glm::vec3(0.1f, 0.1f, 0.1f);

	uint32_t mActiveAnimationID = sBindPoseAnimationID;
	static const uint32_t sBindPoseAnimationID = (uint32_t)-1;

	RasterizerState::SharedPtr mpCullRastState[3]; // 0 = no culling, 1 = backface culling, 2 = frontface culling
	uint32_t mCullMode = 1;

	enum : uint32_t
	{
		Disabled = 0,
		ShowPositions,
		ShowNormals,
		ShowAlbedo,
		ShowLighting
	} mDebugMode = Disabled;

	DepthStencilState::SharedPtr mpNoDepthDS;
	DepthStencilState::SharedPtr mpDepthTestDS;
	BlendState::SharedPtr mpOpaqueBS;

	// G-Buffer
	Fbo::SharedPtr mpGBufferFbo;

	DirectionalLight::SharedPtr mpDirLight;

	float mNearZ = 1e-2f;
	float mFarZ = 1e3f;

	static const std::string skDefaultModel;

	//testing
	void onInitializeTesting(SampleCallbacks* pSample) override;
	void onEndTestFrame(SampleCallbacks* pSample, SampleTest* pSampleTest) override;
	std::vector<uint32_t> mChangeModeFrames;
	std::vector<uint32_t>::iterator mChangeModeIt;
};