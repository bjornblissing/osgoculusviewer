/*
* oculusdevicesdk.cpp
*
*  Created on: Oct 01, 2014
*      Author: Bjorn Blissing
*/
#include "oculusdevicesdk.h"
#include <osg/Notify>
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include "../src/OVR_CAPI_GL.h"
#include "../src/Kernel/OVR_Math.h"

OculusDeviceSDK::OculusDeviceSDK() : m_hmdDevice(0),
	m_frameIndex(0),
	m_nearClip(0.1f),
	m_farClip(10000.0f),
	m_initialized(false),
	m_frameBegin(false),

	m_vSyncEnabled(true),
	m_isLowPersistence(true),
	m_dynamicPrediction(true),
	m_displaySleep(false),
	m_mirrorToWindow(true),

	m_supportsSRGB(true),
	m_pixelLuminanceOverdrive(true),
	m_timewarpEnabled(true),
	m_timewarpNoJitEnabled(false),

	m_positionTrackingEnabled(true)
{}

unsigned int OculusDeviceSDK::getCaps() const {
	unsigned int hmdCaps = (m_vSyncEnabled ? 0 : ovrHmdCap_NoVSync);

	if (m_isLowPersistence)
		hmdCaps |= ovrHmdCap_LowPersistence;

	// ovrHmdCap_DynamicPrediction - enables internal latency feedback
	if (m_dynamicPrediction)
		hmdCaps |= ovrHmdCap_DynamicPrediction;

	// ovrHmdCap_DisplayOff - turns off the display
	if (m_displaySleep)
		hmdCaps |= ovrHmdCap_DisplayOff;

	if (!m_mirrorToWindow)
		hmdCaps |= ovrHmdCap_NoMirrorToWindow;
	return hmdCaps;
}

unsigned int OculusDeviceSDK::getDistortionCaps() const {
	unsigned int distortionCaps = ovrDistortionCap_Chromatic |
			ovrDistortionCap_Vignette;
	if (m_supportsSRGB)
		distortionCaps |= ovrDistortionCap_SRGB;
	if (m_pixelLuminanceOverdrive)
		distortionCaps |= ovrDistortionCap_Overdrive;
	if (m_timewarpEnabled)
		distortionCaps |= ovrDistortionCap_TimeWarp;
	if (m_timewarpNoJitEnabled)
		distortionCaps |= ovrDistortionCap_ProfileNoTimewarpSpinWaits;
	return distortionCaps;
}

void OculusDeviceSDK::calculateViewMatrices() {
	// Calculate the view matrices
	ovrVector3f leftEyeAdjust = m_eyeRenderDesc[0].HmdToEyeViewOffset;
	m_leftEyeAdjust.set(leftEyeAdjust.x, leftEyeAdjust.y, leftEyeAdjust.z);
	ovrVector3f rightEyeAdjust = m_eyeRenderDesc[1].HmdToEyeViewOffset;
	m_rightEyeAdjust.set(rightEyeAdjust.x, rightEyeAdjust.y, rightEyeAdjust.z);

}

void OculusDeviceSDK::calculateProjectionMatrices() {
	// Calculate projection matrices
	ovrMatrix4f leftEyeProjectionMatrix = ovrMatrix4f_Projection(m_eyeRenderDesc[0].Fov, m_nearClip, m_farClip, true);
	// Transpose matrix
	m_leftEyeProjectionMatrix.set(leftEyeProjectionMatrix.M[0][0], leftEyeProjectionMatrix.M[1][0], leftEyeProjectionMatrix.M[2][0], leftEyeProjectionMatrix.M[3][0],
			leftEyeProjectionMatrix.M[0][1], leftEyeProjectionMatrix.M[1][1], leftEyeProjectionMatrix.M[2][1], leftEyeProjectionMatrix.M[3][1],
			leftEyeProjectionMatrix.M[0][2], leftEyeProjectionMatrix.M[1][2], leftEyeProjectionMatrix.M[2][2], leftEyeProjectionMatrix.M[3][2],
			leftEyeProjectionMatrix.M[0][3], leftEyeProjectionMatrix.M[1][3], leftEyeProjectionMatrix.M[2][3], leftEyeProjectionMatrix.M[3][3]);

	ovrMatrix4f rightEyeProjectionMatrix = ovrMatrix4f_Projection(m_eyeRenderDesc[1].Fov, m_nearClip, m_farClip, true);
	// Transpose matrix
	m_rightEyeProjectionMatrix.set(rightEyeProjectionMatrix.M[0][0], rightEyeProjectionMatrix.M[1][0], rightEyeProjectionMatrix.M[2][0], rightEyeProjectionMatrix.M[3][0],
			rightEyeProjectionMatrix.M[0][1], rightEyeProjectionMatrix.M[1][1], rightEyeProjectionMatrix.M[2][1], rightEyeProjectionMatrix.M[3][1],
			rightEyeProjectionMatrix.M[0][2], rightEyeProjectionMatrix.M[1][2], rightEyeProjectionMatrix.M[2][2], rightEyeProjectionMatrix.M[3][2],
			rightEyeProjectionMatrix.M[0][3], rightEyeProjectionMatrix.M[1][3], rightEyeProjectionMatrix.M[2][3], rightEyeProjectionMatrix.M[3][3]);

	float orthoDistance = 0.8f; // 2D is 0.8 meter from camera

	OVR::Vector2f orthoScaleLeft = OVR::Vector2f(1.0f) / OVR::Vector2f(m_eyeRenderDesc[0].PixelsPerTanAngleAtCenter);
	OVR::Vector2f orthoScaleRight = OVR::Vector2f(1.0f) / OVR::Vector2f(m_eyeRenderDesc[1].PixelsPerTanAngleAtCenter);

	ovrMatrix4f leftOrthProjection = ovrMatrix4f_OrthoSubProjection(leftEyeProjectionMatrix, orthoScaleLeft, orthoDistance, m_eyeRenderDesc[0].HmdToEyeViewOffset.x);

	// Transpose matrix
	m_leftEyeOrthoProjectionMatrix.set(leftOrthProjection.M[0][0], leftOrthProjection.M[1][0], leftOrthProjection.M[2][0], leftOrthProjection.M[3][0],
			leftOrthProjection.M[0][1], leftOrthProjection.M[1][1], leftOrthProjection.M[2][1], leftOrthProjection.M[3][1],
			leftOrthProjection.M[0][2], leftOrthProjection.M[1][2], leftOrthProjection.M[2][2], leftOrthProjection.M[3][2],
			leftOrthProjection.M[0][3], leftOrthProjection.M[1][3], leftOrthProjection.M[2][3], leftOrthProjection.M[3][3]);


	ovrMatrix4f rightOrthProjection = ovrMatrix4f_OrthoSubProjection(rightEyeProjectionMatrix, orthoScaleRight, orthoDistance, m_eyeRenderDesc[1].HmdToEyeViewOffset.x);

	// Transpose matrix
	m_rightEyeOrthoProjectionMatrix.set(rightOrthProjection.M[0][0], rightOrthProjection.M[1][0], rightOrthProjection.M[2][0], rightOrthProjection.M[3][0],
			rightOrthProjection.M[0][1], rightOrthProjection.M[1][1], rightOrthProjection.M[2][1], rightOrthProjection.M[3][1],
			rightOrthProjection.M[0][2], rightOrthProjection.M[1][2], rightOrthProjection.M[2][2], rightOrthProjection.M[3][2],
			rightOrthProjection.M[0][3], rightOrthProjection.M[1][3], rightOrthProjection.M[2][3], rightOrthProjection.M[3][3]);
}


void OculusDeviceSDK::printHMDDebugInfo() {
	osg::notify(osg::ALWAYS) << "Product:         " << m_hmdDevice->ProductName << std::endl;
	osg::notify(osg::ALWAYS) << "Manufacturer:    " << m_hmdDevice->Manufacturer << std::endl;
	osg::notify(osg::ALWAYS) << "VendorId:        " << m_hmdDevice->VendorId << std::endl;
	osg::notify(osg::ALWAYS) << "ProductId:       " << m_hmdDevice->ProductId << std::endl;
	osg::notify(osg::ALWAYS) << "SerialNumber:    " << m_hmdDevice->SerialNumber << std::endl;
	osg::notify(osg::ALWAYS) << "FirmwareVersion: " << m_hmdDevice->FirmwareMajor << "." << m_hmdDevice->FirmwareMinor << std::endl;
}

void OculusDeviceSDK::initialize() {
	ovr_Initialize();

	// Enumerate HMD devices
	int numberOfDevices = ovrHmd_Detect();
	osg::notify(osg::DEBUG_INFO) << "Number of connected devices: " << numberOfDevices << std::endl;

	// Get first available HMD
	m_hmdDevice = ovrHmd_Create(0);

	// If no HMD is found try an emulated device
	if (!m_hmdDevice) {
		osg::notify(osg::WARN) << "Warning: No device could be found. Creating emulated device " << std::endl;
		m_hmdDevice = ovrHmd_CreateDebug(ovrHmd_DK1);
		ovrHmd_ResetFrameTiming(m_hmdDevice, 0);
	}

	if (m_hmdDevice) {
		printHMDDebugInfo();

		// Get more details about the HMD.
		if (m_hmdDevice->HmdCaps & ovrHmdCap_ExtendDesktop)	{
			m_resolution = m_hmdDevice->Resolution;
		} else	{
			// In Direct App-rendered mode, we can use smaller window size,
			// as it can have its own contents and isn't tied to the buffer.
			m_resolution.w = 1100;
			m_resolution.h = 618;
		}

		// Compute recommended render texture size
		float pixelsPerDisplayPixel = 1.0f; // Decrease this value to scale the size on render texture on lower performance hardware. Values above 1.0 is unnecessary.

		ovrSizei recommenedLeftTextureSize = ovrHmd_GetFovTextureSize(m_hmdDevice, ovrEye_Left, m_hmdDevice->DefaultEyeFov[0], pixelsPerDisplayPixel);
		ovrSizei recommenedRightTextureSize = ovrHmd_GetFovTextureSize(m_hmdDevice, ovrEye_Right, m_hmdDevice->DefaultEyeFov[1], pixelsPerDisplayPixel);

		m_eyeFov[0] = m_hmdDevice->DefaultEyeFov[0];
		m_eyeFov[1] = m_hmdDevice->DefaultEyeFov[1];

		// Initialize ovrEyeRenderDesc struct.
		m_eyeRenderDesc[0] = ovrHmd_GetRenderDesc(m_hmdDevice, ovrEye_Left, m_eyeFov[0]);
		m_eyeRenderDesc[1] = ovrHmd_GetRenderDesc(m_hmdDevice, ovrEye_Right, m_eyeFov[1]);

		// Compute size of render target
		m_renderTargetSize.w = recommenedLeftTextureSize.w + recommenedRightTextureSize.w;
		m_renderTargetSize.h = osg::maximum(recommenedLeftTextureSize.h, recommenedRightTextureSize.h);
	}
}

void OculusDeviceSDK::initializeTexture(osg::ref_ptr<osg::GraphicsContext> context) {
	m_leftTexture = new OculusTexture2D;
	m_leftTexture->setContext(context);
	m_leftTexture->setViewPort(0, 0, m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_leftTexture->setTextureSize(m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_leftTexture->setInternalFormat(GL_RGBA);

	m_rightTexture = new OculusTexture2D;
	m_rightTexture->setContext(context);
	m_rightTexture->setViewPort(m_renderTargetSize.w / 2, 0, m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_rightTexture->setTextureSize(m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_rightTexture->setInternalFormat(GL_RGBA);

	osg::State* state = context->getState();
	m_leftTexture->allocateMipmapLevels();
	m_rightTexture->allocateMipmapLevels();
	m_leftTexture->apply(*state);
	m_rightTexture->apply(*state);
}

void OculusDeviceSDK::setupSlaveCameras(osgViewer::View* view) {
	osg::GraphicsContext* context = view->getCamera()->getGraphicsContext();

	osg::ref_ptr<osg::Camera> cameraRTTLeft = createRTTCamera(m_leftTexture, 0, context);
	osg::ref_ptr<osg::Camera> cameraRTTRight = createRTTCamera(m_rightTexture, 1, context);
	
	// Add RTT cameras as slaves, specifying offsets for the projection
	view->addSlave(cameraRTTLeft.get(),
		projectionOffsetMatrixLeft(),
		viewMatrixLeft(),
		true);
	view->addSlave(cameraRTTRight.get(),
		projectionOffsetMatrixRight(),
		viewMatrixRight(),
		true);
}

void OculusDeviceSDK::configureRendering(HWND window, HDC dc, int backBufferMultisample) {
	// Hmd caps.
	unsigned int hmdCaps = getCaps();

	ovrHmd_SetEnabledCaps(m_hmdDevice, hmdCaps);

	unsigned int distortionCaps = getDistortionCaps();

	// Configure OpenGL
	ovrGLConfig cfg;
	cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	cfg.OGL.Header.BackBufferSize = m_resolution;
	cfg.OGL.Header.Multisample = backBufferMultisample;
	cfg.OGL.Window = window;
	cfg.OGL.DC = dc;
	ovrBool result = ovrHmd_ConfigureRendering(m_hmdDevice, &cfg.Config, distortionCaps, m_eyeFov, m_eyeRenderDesc);

	if (!result) {
		return;
	}

	unsigned int sensorCaps = ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection;
	if (m_positionTrackingEnabled) {
		sensorCaps |= ovrTrackingCap_Position;
	}

	ovrHmd_ConfigureTracking(m_hmdDevice, sensorCaps, 0);

	calculateViewMatrices();

	calculateProjectionMatrices();
}

void OculusDeviceSDK::attachToWindow(HWND window) {
	// Direct rendering from a window handle to the Hmd.
	// Not required if ovrHmdCap_ExtendDesktop flag is set.
	if (m_hmdDevice->HmdCaps & ovrHmdCap_ExtendDesktop) {
		osg::notify(osg::DEBUG_INFO) << "No need to attach to when using extended desktop" << std::endl;
	} else {
		ovrHmd_AttachToWindow(m_hmdDevice, window, NULL, NULL);
	}
}

void OculusDeviceSDK::getEyePose() {
	if (m_hmdDevice && m_frameBegin) {
		m_renderPose[0] = ovrHmd_GetHmdPosePerEye(m_hmdDevice, ovrEye_Left);
		m_renderPose[1] = ovrHmd_GetHmdPosePerEye(m_hmdDevice, ovrEye_Right);
	}
}

void OculusDeviceSDK::beginFrame() {
	if (m_hmdDevice && m_initialized) {
		ovrHmd_BeginFrame(m_hmdDevice, 0);
		m_frameBegin = true;
	}
}

void OculusDeviceSDK::endFrame() {
	if (m_hmdDevice && m_frameBegin) {
		ovrTexture eyeTexture[2];
		eyeTexture[0] = m_leftTexture->getOvrTexture();
		eyeTexture[1] = m_rightTexture->getOvrTexture();
		ovrHmd_EndFrame(m_hmdDevice, m_renderPose, eyeTexture);
	}
}

unsigned int OculusDeviceSDK::screenResolutionWidth() const
{
	return m_resolution.w;
}

unsigned int OculusDeviceSDK::screenResolutionHeight() const
{
	return m_resolution.h;
}

osg::Matrix OculusDeviceSDK::projectionOffsetMatrixLeft() const
{
	osg::Matrix projectionOffsetMatrix;
	float offset = m_leftEyeProjectionMatrix(2, 0);
	projectionOffsetMatrix.makeTranslate(osg::Vec3(-offset, 0.0, 0.0));
	return projectionOffsetMatrix;
}

osg::Matrix OculusDeviceSDK::projectionOffsetMatrixRight() const
{
	osg::Matrix projectionOffsetMatrix;
	float offset = m_rightEyeProjectionMatrix(2, 0);
	projectionOffsetMatrix.makeTranslate(osg::Vec3(-offset, 0.0, 0.0));
	return projectionOffsetMatrix;
}

osg::Matrix OculusDeviceSDK::viewMatrixLeft() const
{
	osg::Matrix viewMatrix;
	viewMatrix.makeTranslate(m_leftEyeAdjust);
	return viewMatrix;
}

osg::Matrix OculusDeviceSDK::viewMatrixRight() const
{
	osg::Matrix viewMatrix;
	viewMatrix.makeTranslate(m_rightEyeAdjust);
	return viewMatrix;
}

OculusDeviceSDK::~OculusDeviceSDK()
{
	ovrHmd_Destroy(m_hmdDevice);
	ovr_Shutdown();
}

osg::GraphicsContext::Traits* OculusDeviceSDK::graphicsContextTraits() const {
	// Create screen with match the Oculus Rift resolution
	osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();

	if (!wsi) {
		osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
		return 0;
	}

	// Get the screen identifiers set in environment variable DISPLAY
	osg::GraphicsContext::ScreenIdentifier si;
	si.readDISPLAY();

	// If displayNum has not been set, reset it to 0.
	if (si.displayNum < 0) si.displayNum = 0;

	// If screenNum has not been set, reset it to 0.
	if (si.screenNum < 0) si.screenNum = 0;

	unsigned int width, height;
	wsi->getScreenResolution(si, width, height);

	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->hostName = si.hostName;
	traits->screenNum = si.screenNum;
	traits->displayNum = si.displayNum;
	traits->windowDecoration = false;
	traits->x = m_hmdDevice->WindowsPos.x;
	traits->y = m_hmdDevice->WindowsPos.y;
	traits->width = screenResolutionWidth();
	traits->height = screenResolutionHeight();
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->samples = 1;
	traits->vsync = true; // VSync should always be enabled for Oculus Rift applications

	return traits.release();
}

osg::Camera* OculusDeviceSDK::createRTTCamera(osg::Texture* texture, int order, osg::GraphicsContext* gc) const
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setClearColor(osg::Vec4(0.2f, 0.8f, 0.4f, 1.0f));
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->setDrawBuffer(GL_FRONT);
	camera->setReadBuffer(GL_FRONT);
	camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	camera->setRenderOrder(osg::Camera::PRE_RENDER, order);
	camera->setAllowEventFocus(false);
	camera->setReferenceFrame(osg::Camera::RELATIVE_RF);
	camera->setGraphicsContext(gc);
	
	if (texture) {
		texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		camera->setViewport(0, 0, texture->getTextureWidth(), texture->getTextureHeight());
		camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, 0, false, 4, 4);
	}

	return camera.release();
}

void OculusTexture2D::setViewPort(int x, int y, int w, int h) {
	m_viewPort.Pos.x = x;
	m_viewPort.Pos.y = y;
	m_viewPort.Size.w = w;
	m_viewPort.Size.h = h;
}

ovrTexture OculusTexture2D::getOvrTexture()
{
	ovrTexture texture;
	OVR::Sizei newRTSize(getTextureWidth(), getTextureHeight());

	ovrGLTextureData* texData = reinterpret_cast<ovrGLTextureData*>(&texture);
	texData->Header.API = ovrRenderAPI_OpenGL;
	texData->Header.TextureSize = newRTSize;
	texData->Header.RenderViewport = m_viewPort;
	unsigned int contextID = m_context->getState()->getContextID();
	texData->TexId = getTextureObject(contextID)->id();
	return texture;
}

OculusDrawable::OculusDrawable(const OculusDrawable& drawable, const osg::CopyOp& copyop) :
Drawable(drawable, copyop), m_oculusDevice(drawable.m_oculusDevice), m_view(drawable.m_view) {}

void OculusDrawable::drawImplementation(osg::RenderInfo& info) const {	
	if (!m_oculusDevice->initialized()) {
		osg::State* state = info.getState();
		osg::GraphicsContext* context = state->getGraphicsContext();
		if (context->isRealized() && context->isCurrent()) {
			osg::ref_ptr<osgViewer::GraphicsWindowWin32> wdc = dynamic_cast<osgViewer::GraphicsWindowWin32*>(context);
			if (wdc.valid()) {
				HWND window = wdc->getHWND();
				HDC dc = wdc->getHDC();
				m_oculusDevice->initializeTexture(wdc.get());
				m_oculusDevice->setupSlaveCameras(m_view.get());
				m_oculusDevice->configureRendering(window, dc , 1);
				m_oculusDevice->attachToWindow(window);
				m_oculusDevice->setInitialized(true);
			}
		}
	}
}
