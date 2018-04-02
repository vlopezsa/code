#pragma once
#include "Falcor.h"
#include "SampleTest.h"

using namespace Falcor;


class Deferred : public SampleTest
{
public:
	~Deferred();

	void onLoad() override;
	void onFrameRender() override;
	void onShutdown() override;
	void onResizeSwapChain() override;
	bool onKeyEvent(const KeyboardEvent& keyEvent) override;
	bool onMouseEvent(const MouseEvent& mouseEvent) override;
	void onGuiRender() override;
private:
	void reset();
	void loadModel();
	void loadModelFromFile(const std::string& filename);
	void loadScene();
	void renderModelUiElements();

	void initScene();

	Model::SharedPtr mpModel = nullptr;
	FirstPersonCameraController mFirstPersonCameraController;
	Sampler::SharedPtr mpLinearSampler;

	GraphicsProgram::SharedPtr mpDeferredPassProgram;
	GraphicsVars::SharedPtr mpDeferredVars;

	GraphicsVars::SharedPtr mpLightingVars;
	FullScreenPass::UniquePtr mpLightingPass;

	Scene::SharedPtr mpScene = nullptr;
	SceneRenderer::SharedPtr mpRenderer = nullptr;

	float mAspectRatio = 0;

	CameraController& getActiveCameraController();

	Camera::SharedPtr mpCamera;

	bool mAnimate = false;
	bool mGenerateTangentSpace = true;
	glm::vec3 mAmbientIntensity = glm::vec3(0.1f, 0.1f, 0.1f);

	uint32_t mActiveAnimationID = sBindPoseAnimationID;
	static const uint32_t sBindPoseAnimationID = (uint32_t)-1;

	RasterizerState::SharedPtr mpCullRastState;

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

	//testing
	void onInitializeTesting() override;
	void onEndTestFrame() override;
	std::vector<uint32_t> mChangeModeFrames;
	std::vector<uint32_t>::iterator mChangeModeIt;
};