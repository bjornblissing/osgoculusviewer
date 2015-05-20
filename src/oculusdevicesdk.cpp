/*
* oculusdevicesdk.cpp
*
*  Created on: Oct 01, 2014
*      Author: Bjorn Blissing
*/
#include "oculusdevicesdk.h"
#include <osg/Notify>
#include <OVR_CAPI_GL.h>
#include <Extras/OVR_Math.h>

#ifdef _WIN32
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#endif


/* Public functions */
OculusDeviceSDK::OculusDeviceSDK(float nearClip, float farClip, const float pixelsPerDisplayPixel, const float worldUnitsPerMetre) : m_hmdDevice(0),
	m_view(0),
	m_warning(0),
	m_pixelsPerDisplayPixel(pixelsPerDisplayPixel),
	m_worldUnitsPerMetre(worldUnitsPerMetre),
	m_nearClip(nearClip),
	m_farClip(farClip),
	m_initialized(false),
	m_frameBegin(false),

	m_vSyncEnabled(true),
	m_isLowPersistence(true),
	m_dynamicPrediction(true),
	m_displaySleep(false),
	m_mirrorToWindow(true),

	m_supportsSRGB(false),
	m_pixelLuminanceOverdrive(true),
	m_timewarpEnabled(true),

	m_positionTrackingEnabled(true),
	m_directMode(true)
{
	trySetProcessAsHighPriority();
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

	if (!m_hmdDevice) {
		osg::notify(osg::WARN) << "Warning: No device could be found. and failed to create an emulated device " << std::endl;
		return;
	}

	printHMDDebugInfo();

	// Get more details about the HMD.
	if (m_hmdDevice->HmdCaps & ovrHmdCap_ExtendDesktop)	{
		m_resolution = m_hmdDevice->Resolution;
	}
	else	{
		// In Direct App-rendered mode, we can use smaller window size,
		// as it can have its own contents and isn't tied to the buffer.
		m_resolution.w = 1100;
		m_resolution.h = 618;
	}

	// Compute recommended render texture size
	ovrSizei recommenedLeftTextureSize = ovrHmd_GetFovTextureSize(m_hmdDevice, ovrEye_Left, m_hmdDevice->DefaultEyeFov[0], m_pixelsPerDisplayPixel);
	ovrSizei recommenedRightTextureSize = ovrHmd_GetFovTextureSize(m_hmdDevice, ovrEye_Right, m_hmdDevice->DefaultEyeFov[1], m_pixelsPerDisplayPixel);

	// Compute size of render target
	m_renderTargetSize.w = recommenedLeftTextureSize.w + recommenedRightTextureSize.w;
	m_renderTargetSize.h = osg::maximum(recommenedLeftTextureSize.h, recommenedRightTextureSize.h);

	// Add health and safety warning
	m_warning = new OculusHealthAndSafetyWarning();

	initializeEyeRenderDesc();
}

void OculusDeviceSDK::resetSensorOrientation() const {
	ovrHmd_RecenterPose(m_hmdDevice);
}

void OculusDeviceSDK::toggleMirrorToWindow() {
	unsigned int hmdCaps = ovrHmd_GetEnabledCaps(m_hmdDevice);
	hmdCaps ^= ovrHmdCap_NoMirrorToWindow;
	ovrHmd_SetEnabledCaps(m_hmdDevice, hmdCaps);
}

void OculusDeviceSDK::toggleLowPersistence() {
	unsigned int hmdCaps = ovrHmd_GetEnabledCaps(m_hmdDevice);
	hmdCaps ^= ovrHmdCap_LowPersistence;
	ovrHmd_SetEnabledCaps(m_hmdDevice, hmdCaps);
}

void OculusDeviceSDK::toggleDynamicPrediction() {
	unsigned int hmdCaps = ovrHmd_GetEnabledCaps(m_hmdDevice);
	hmdCaps ^= ovrHmdCap_DynamicPrediction;
	ovrHmd_SetEnabledCaps(m_hmdDevice, hmdCaps);
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
	if (si.displayNum < 0) {
		si.displayNum = 0;
		osg::notify(osg::INFO) << "Couldn't get display number, setting to 0" << std::endl;
	}

	// If screenNum has not been set, reset it to 0.
	if (si.screenNum < 0) {
		si.screenNum = 0;
		osg::notify(osg::INFO) << "Couldn't get screen number, setting to 0" << std::endl;
	}

	unsigned int width, height;
	wsi->getScreenResolution(si, width, height);

	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->hostName = si.hostName;
	traits->screenNum = si.screenNum;
	traits->displayNum = si.displayNum;
	traits->windowDecoration = false;
	traits->x = m_hmdDevice->WindowsPos.x;
	traits->y = m_hmdDevice->WindowsPos.y;
	traits->width = m_resolution.w;
	traits->height = m_resolution.h;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->samples = 1;
	traits->vsync = true; // VSync should always be enabled for Oculus Rift applications

	return traits.release();
}

void OculusDeviceSDK::beginFrame() {
	if (!m_hmdDevice || !m_initialized) {
		if (!m_hmdDevice) osg::notify(osg::WARN) << "Tried to use ovrHmd_BeginFrame without having a valid hmdDevice" << std::endl;
		if (!m_initialized) osg::notify(osg::WARN) << "Tried to use ovrHmd_BeginFrame without being initialized" << std::endl;
		return;
	}

	ovrHmd_BeginFrame(m_hmdDevice, 0);

	m_renderPose[0] = ovrHmd_GetHmdPosePerEye(m_hmdDevice, ovrEye_Left);
	updatePose(m_renderPose[0], m_cameraRTTLeft.get(), viewMatrixLeft());

	m_renderPose[1] = ovrHmd_GetHmdPosePerEye(m_hmdDevice, ovrEye_Right);
	updatePose(m_renderPose[1], m_cameraRTTRight.get(), viewMatrixRight());

	m_frameBegin = true;
}

void OculusDeviceSDK::endFrame() {
	if (!m_hmdDevice || !m_frameBegin) {
		if (!m_hmdDevice) osg::notify(osg::WARN) << "Tried to use ovrHmd_EndFrame without having a valid hmdDevice" << std::endl;
		if (!m_frameBegin) osg::notify(osg::WARN) << "Mismatching BeginFrame / EndFrame" << std::endl;
		return;
	}

	ovrTexture eyeTexture[2];
	eyeTexture[0] = m_leftTexture->getOvrTexture();
	eyeTexture[1] = m_rightTexture->getOvrTexture();

	ovrHmd_EndFrame(m_hmdDevice, m_renderPose, eyeTexture);
	m_frameBegin = false;
}

bool OculusDeviceSDK::getHealthAndSafetyDisplayState() {
	ovrHSWDisplayState hswDisplayState;
 	ovrHmd_GetHSWDisplayState(m_hmdDevice, &hswDisplayState);
	return (hswDisplayState.Displayed != 0);
}

bool OculusDeviceSDK::tryDismissHealthAndSafetyDisplay() {
	return (ovrHmd_DismissHSWDisplay(m_hmdDevice) != 0);
}

/* Protected functions */
OculusDeviceSDK::~OculusDeviceSDK()
{
	ovrHmd_Destroy(m_hmdDevice);
	ovr_Shutdown();
}

int OculusDeviceSDK::renderOrder(Eye eye) const {
	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; ++eyeIndex) {
		ovrEyeType ovrEye = m_hmdDevice->EyeRenderOrder[eyeIndex];
		if (ovrEye == ovrEye_Left && eye == LEFT) {
			return eyeIndex;
		}
		if (ovrEye == ovrEye_Right && eye == RIGHT) {
			return eyeIndex;
		}
	}
	return 0;
}

void OculusDeviceSDK::printHMDDebugInfo() {
	osg::notify(osg::ALWAYS) << "Product:         " << m_hmdDevice->ProductName << std::endl;
	osg::notify(osg::ALWAYS) << "Manufacturer:    " << m_hmdDevice->Manufacturer << std::endl;
	osg::notify(osg::ALWAYS) << "VendorId:        " << m_hmdDevice->VendorId << std::endl;
	osg::notify(osg::ALWAYS) << "ProductId:       " << m_hmdDevice->ProductId << std::endl;
	osg::notify(osg::ALWAYS) << "SerialNumber:    " << m_hmdDevice->SerialNumber << std::endl;
	osg::notify(osg::ALWAYS) << "FirmwareVersion: " << m_hmdDevice->FirmwareMajor << "." << m_hmdDevice->FirmwareMinor << std::endl;
}

void OculusDeviceSDK::initializeEyeRenderDesc() {
	m_eyeFov[0] = m_hmdDevice->DefaultEyeFov[0];
	m_eyeFov[1] = m_hmdDevice->DefaultEyeFov[1];

	// Initialize ovrEyeRenderDesc struct.
	m_eyeRenderDesc[0] = ovrHmd_GetRenderDesc(m_hmdDevice, ovrEye_Left, m_eyeFov[0]);
	m_eyeRenderDesc[1] = ovrHmd_GetRenderDesc(m_hmdDevice, ovrEye_Right, m_eyeFov[1]);
}

void OculusDeviceSDK::calculateEyeAdjustment() {
	// Calculate the view matrices
	ovrVector3f leftEyeAdjust = m_eyeRenderDesc[0].HmdToEyeViewOffset;
	m_leftEyeAdjust.set(leftEyeAdjust.x, leftEyeAdjust.y, leftEyeAdjust.z);
	m_leftEyeAdjust *= m_worldUnitsPerMetre;
	ovrVector3f rightEyeAdjust = m_eyeRenderDesc[1].HmdToEyeViewOffset;
	m_rightEyeAdjust *= m_worldUnitsPerMetre;
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
	unsigned int distortionCaps = ovrDistortionCap_Vignette;
	if (m_supportsSRGB)
		distortionCaps |= ovrDistortionCap_SRGB;
	if (m_pixelLuminanceOverdrive)
		distortionCaps |= ovrDistortionCap_Overdrive;
	if (m_timewarpEnabled)
		distortionCaps |= ovrDistortionCap_TimeWarp;
	return distortionCaps;
}

osg::Matrix OculusDeviceSDK::projectionMatrixCenter() const
{
	osg::Matrix projectionMatrixCenter;
	projectionMatrixCenter = m_leftEyeProjectionMatrix.operator*(0.5) + m_rightEyeProjectionMatrix.operator*(0.5);
	return projectionMatrixCenter;
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

void OculusDeviceSDK::updatePose(const ovrPosef& pose, osg::Camera* camera, osg::Matrix viewMatrix) {
	osg::Vec3 position(-pose.Position.x, -pose.Position.y, -pose.Position.z);
	osg::Quat orientation(pose.Orientation.x, pose.Orientation.y, pose.Orientation.z, -pose.Orientation.w);
	osg::Matrix viewOffset = viewMatrixLeft();
	viewOffset.preMultRotate(orientation);
	viewOffset.preMultTranslate(position);
	// Nasty hack to update the view offset for each of the slave cameras
	// There doesn't seem to be an accessor for this, fortunately the offsets are public
	m_view->findSlaveForCamera(camera)->_viewOffset = viewOffset;

	// Handle health and safety warning
	if (m_warning.valid()) {
		// Set warning relative main camera
		m_warning.get()->updatePosition(m_view->getCamera()->getInverseViewMatrix(), position, orientation);
	}
}

void OculusDeviceSDK::initializeTextures(osg::ref_ptr<osg::GraphicsContext> context) {
	m_leftTexture = new OculusTexture2D;
	m_leftTexture->setContext(context);
	m_leftTexture->setViewPort(0, 0, m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_leftTexture->setTextureSize(m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_leftTexture->setInternalFormat(GL_RGBA);

	m_rightTexture = new OculusTexture2D;
	m_rightTexture->setContext(context);
	m_rightTexture->setViewPort(0, 0, m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_rightTexture->setTextureSize(m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_rightTexture->setInternalFormat(GL_RGBA);

	m_leftDepthTexture = new OculusTexture2D;
	m_leftDepthTexture->setContext(context);
	m_leftDepthTexture->setViewPort(0, 0, m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_leftDepthTexture->setTextureSize(m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_leftDepthTexture->setInternalFormat(GL_DEPTH_COMPONENT32);
	m_leftDepthTexture->setSourceFormat(GL_DEPTH_COMPONENT);
	m_leftDepthTexture->setSourceType(GL_FLOAT);

	m_rightDepthTexture = new OculusTexture2D;
	m_rightDepthTexture->setContext(context);
	m_rightDepthTexture->setViewPort(0, 0, m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_rightDepthTexture->setTextureSize(m_renderTargetSize.w / 2, m_renderTargetSize.h);
	m_rightDepthTexture->setInternalFormat(GL_DEPTH_COMPONENT32);
	m_rightDepthTexture->setSourceFormat(GL_DEPTH_COMPONENT);
	m_rightDepthTexture->setSourceType(GL_FLOAT);
}

#if _WIN32
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

	calculateEyeAdjustment();

	calculateProjectionMatrices();
}

#elif __linux__
void OculusDeviceSDK::configureRendering(Display* disp, int backBufferMultisample) {
	// Hmd caps.
	unsigned int hmdCaps = getCaps();

	ovrHmd_SetEnabledCaps(m_hmdDevice, hmdCaps);

	unsigned int distortionCaps = getDistortionCaps();

	// Configure OpenGL
	ovrGLConfig cfg;
	cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	cfg.OGL.Header.BackBufferSize = m_resolution;
	cfg.OGL.Header.Multisample = backBufferMultisample;
	cfg.OGL.Disp = disp;

	ovrBool result = ovrHmd_ConfigureRendering(m_hmdDevice, &cfg.Config, distortionCaps, m_eyeFov, m_eyeRenderDesc);

	if (!result) {
		osg::notify(osg::WARN) << "Error configuring OVR rendering!" << std::endl;
		return;
	}

	unsigned int sensorCaps = ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection;
	if (m_positionTrackingEnabled) {
		sensorCaps |= ovrTrackingCap_Position;
	}

	ovrHmd_ConfigureTracking(m_hmdDevice, sensorCaps, 0);

	calculateEyeAdjustment();

	calculateProjectionMatrices();
}
#endif

void OculusDeviceSDK::setupSlaveCameras(osg::GraphicsContext* context, osgViewer::View* view) {
	m_view = view;
	m_cameraRTTLeft = createRTTCamera(m_leftTexture, m_leftDepthTexture, renderOrder(Eye::LEFT), context);
	m_cameraRTTRight = createRTTCamera(m_rightTexture, m_rightDepthTexture, renderOrder(Eye::RIGHT), context);

	// Add RTT cameras as slaves, specifying offsets for the projection
	m_view->addSlave(m_cameraRTTLeft.get(),
		projectionOffsetMatrixLeft(),
		viewMatrixLeft(),
		true);
	m_view->addSlave(m_cameraRTTRight.get(),
		projectionOffsetMatrixRight(),
		viewMatrixRight(),
		true);
}

osg::Camera* OculusDeviceSDK::createRTTCamera(osg::Texture* texture, osg::Texture* depthTexture, int order, osg::GraphicsContext* gc) const
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
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
		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		camera->setViewport(0, 0, texture->getTextureWidth(), texture->getTextureHeight());
		camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, 0, false, 4, 4);
	}

	if (depthTexture) {
		depthTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		depthTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		depthTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		depthTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		camera->setViewport(0, 0, depthTexture->getTextureWidth(), depthTexture->getTextureHeight());
		camera->attach(osg::Camera::DEPTH_BUFFER, depthTexture);
	}

	return camera.release();
}

bool OculusDeviceSDK::attachToWindow(osg::ref_ptr<osg::GraphicsContext> gc) {
	// Do not attach if we are using extended desktop
	if (m_hmdDevice->HmdCaps & ovrHmdCap_ExtendDesktop) {
		applyExtendedModeSettings();
		return false;
	}

	// Direct rendering from a window handle to the Rift
#ifdef _WIN32
	osgViewer::GraphicsHandleWin32* windowsContext = dynamic_cast<osgViewer::GraphicsHandleWin32*>(gc.get());

	if (windowsContext) {
		HWND window = windowsContext->getHWND();
		m_directMode = (ovrHmd_AttachToWindow(m_hmdDevice, window, NULL, NULL) > 0) ? true : false;
		return m_directMode;
	}
	else {
		osg::notify(osg::FATAL) << "Win32 Graphics Context Casting is unsuccessful" << std::endl;
		return false;
	}
#endif
}

void OculusDeviceSDK::trySetProcessAsHighPriority() const {
	// Require at least 4 processors, otherwise the process could occupy the machine.
	if (OpenThreads::GetNumberOfProcessors() >= 4) {
#ifdef _WIN32
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif
	}
}

void OculusDeviceSDK::applyExtendedModeSettings() const {
	// No need to disable desktop composition
}


/* OculusTexture2D */
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


/* OculusRealizeOperation */
void OculusRealizeOperation::operator() (osg::GraphicsContext* gc) {
	if (!m_realized) {
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
		gc->makeCurrent();

#if _WIN32
		osg::ref_ptr<osgViewer::GraphicsWindowWin32> wdc = dynamic_cast<osgViewer::GraphicsWindowWin32*>(gc);

		if (!wdc.valid()) {
			return;
		}

		HWND window = wdc->getHWND();
		HDC dc = wdc->getHDC();
		m_oculusDevice->initializeTextures(wdc.get());
		m_oculusDevice->configureRendering(window, dc, 1);
	
#elif __linux__
		osg::ref_ptr<osgViewer::GraphicsWindowX11> ldc = dynamic_cast<osgViewer::GraphicsWindowX11*>(gc);
		if (!ldc.valid()) {
			return;
		}
		Display* disp = ldc->getDisplayToUse();

		m_oculusDevice->initializeTextures(ldc.get());
		m_oculusDevice->configureRendering(disp, 1);

#endif
		osg::ref_ptr<osg::Camera> camera = m_view->getCamera();
		camera->setName("Main");

		// master projection matrix
		camera->setProjectionMatrix(m_oculusDevice->projectionMatrixCenter());

		// Use sky light instead of headlight to avoid light changes when head movements
		m_view->setLightingMode(osg::View::SKY_LIGHT);

		m_oculusDevice->setupSlaveCameras(gc, m_view.get());
		m_oculusDevice->attachToWindow(gc);
		m_oculusDevice->setInitialized(true);
	}
	m_realized = true;
}