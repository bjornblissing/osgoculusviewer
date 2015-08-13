/*
 * oculusdevice.cpp
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#include "oculusdevice.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <osg/Geometry>
#include <osgViewer/Renderer>
#include <osgViewer/GraphicsWindow>


void OculusPreDrawCallback::operator()(osg::RenderInfo& renderInfo) const {
	osg::State& state = *renderInfo.getState();
	const unsigned int ctx = state.getContextID();
	
	if (!m_textureBuffer->isFboIdInitialized()) {
		osg::Camera* camera = renderInfo.getCurrentCamera();
		osgViewer::Renderer *camRenderer = (dynamic_cast<osgViewer::Renderer*>(camera->getRenderer()));
		if (camRenderer != NULL) {
			osgUtil::SceneView* sceneView = camRenderer->getSceneView(0);
			if (sceneView != NULL) {
				osgUtil::RenderStage* renderStage = sceneView->getRenderStage();
				if (renderStage != NULL) {
					osg::FrameBufferObject* fbo = renderStage->getFrameBufferObject();
					GLuint fboId = fbo->getHandle(ctx);
					m_textureBuffer->initializeFboId(fboId);
				}
			}
		}
	}

	m_textureBuffer->advanceIndex();

	const osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(ctx, true);
	m_textureBuffer->setRenderSurface(fbo_ext);
	m_depthBuffer->setRenderSurface(fbo_ext);
}

/* Public functions */
OculusTextureBuffer::OculusTextureBuffer(const ovrHmd& hmd, osg::ref_ptr<osg::State> state, const ovrSizei& size) : m_textureSet(0), 
	m_texture(0), 
	m_textureSize(osg::Vec2i(size.w, size.h)), 
	m_contextId(0), 
	m_fboId(0), 
	m_fboIdInitialized(false) 
{
	m_textureSize.set(size.w, size.h);
	if (ovrHmd_CreateSwapTextureSetGL(hmd, GL_RGBA, size.w, size.h, &m_textureSet) == ovrSuccess) {
		// Assign textures to OSG textures
		for (int i = 0; i < m_textureSet->TextureCount; ++i)
		{
			ovrGLTexture* tex = (ovrGLTexture*)&m_textureSet->Textures[i];

			GLuint handle = tex->OGL.TexId;
			m_texture = new osg::Texture2D();

			osg::ref_ptr<osg::Texture::TextureObject> textureObject = new osg::Texture::TextureObject(m_texture.get(), handle, GL_TEXTURE_2D);

			textureObject->setAllocated(true);

			m_texture->setTextureObject(state->getContextID(), textureObject.get());

			m_texture->apply(*state.get());

			m_texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
			m_texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
			m_texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
			m_texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

			m_texture->setTextureSize(tex->Texture.Header.TextureSize.w, tex->Texture.Header.TextureSize.h);
			m_texture->setSourceFormat(GL_RGBA);
			m_texture->apply(*state.get());
			m_contextId = state->getContextID();

			osg::notify(osg::DEBUG_INFO) << "Successfully created the swap texture!" << std::endl;
		}
	}
	else {
		osg::notify(osg::WARN) << "Warning: Unable to create swap texture set! " << std::endl;
		return;
	}
}

void OculusTextureBuffer::setRenderSurface(const osg::FBOExtensions* fbo_ext) {
	ovrGLTexture* tex = (ovrGLTexture*)&m_textureSet->Textures[m_textureSet->CurrentIndex];
	fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_fboId);
	fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex->OGL.TexId, 0);
	
}

void OculusDepthBuffer::setRenderSurface(const osg::FBOExtensions* fbo_ext) {
	fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_texId, 0);
}

OculusDepthBuffer::OculusDepthBuffer(const ovrSizei& size, osg::ref_ptr<osg::State> state) : m_texture(0) {
	m_textureSize.set(size.w, size.h);
	m_texture = new osg::Texture2D();
	m_texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	m_texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	m_texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_texture->setSourceFormat(GL_DEPTH_COMPONENT); 
	m_texture->setSourceType(GL_UNSIGNED_INT);
	m_texture->setInternalFormat(GL_DEPTH_COMPONENT24);
	m_texture->setTextureWidth(size.w);
	m_texture->setTextureHeight(size.h);
	m_texture->setBorderWidth(0);
	m_texture->apply(*state.get());
	unsigned int contextID = state->getContextID();
	m_texId = m_texture->getTextureObject(contextID)->id();
}

/* Public functions */
OculusDevice::OculusDevice(float nearClip, float farClip, const float pixelsPerDisplayPixel, const float worldUnitsPerMetre) : m_hmdDevice(0),
m_pixelsPerDisplayPixel(pixelsPerDisplayPixel),
m_worldUnitsPerMetre(worldUnitsPerMetre),
m_position(osg::Vec3(0.0f, 0.0f, 0.0f)),
m_orientation(osg::Quat(0.0f, 0.0f, 0.0f, 1.0f)),
m_nearClip(nearClip), m_farClip(farClip)
{
	trySetProcessAsHighPriority();
	if (ovr_Initialize(nullptr) != ovrSuccess) {
		osg::notify(osg::WARN) << "Warning: Unable to initialize the Oculus library!" << std::endl;
		return;
	}

	// Enumerate HMD devices
	int numberOfDevices = ovrHmd_Detect();
	osg::notify(osg::DEBUG_INFO) << "Number of connected devices: " << numberOfDevices << std::endl;

	// Get first available HMD
	ovrResult result = ovrHmd_Create(0, &m_hmdDevice);
	if (result != ovrSuccess)
	{
		// If no HMD is found try an emulated device
		osg::notify(osg::WARN) << "Warning: No device could be found. Creating emulated device " << std::endl;
		result = ovrHmd_CreateDebug(ovrHmd_DK2, &m_hmdDevice);
	}

	if (result != ovrSuccess) {
		osg::notify(osg::WARN) << "Warning: No device could be found and couldn't create an emulated device " << std::endl;
		return;
	}

	// Print information about device
	printHMDDebugInfo();

	// Get more details about the HMD.
	m_resolution = m_hmdDevice->Resolution;
}

void OculusDevice::createRenderBuffers(osg::ref_ptr<osg::State> state) {
	// Compute recommended render texture size
	if (m_pixelsPerDisplayPixel > 1.0f) {
		osg::notify(osg::WARN) << "Warning: Pixel per display pixel is set to a value higher than 1.0." << std::endl;
	}
	for (int i = 0; i<2; i++)
	{
		ovrSizei recommenedTextureSize = ovrHmd_GetFovTextureSize(m_hmdDevice, (ovrEyeType)i, m_hmdDevice->DefaultEyeFov[i], m_pixelsPerDisplayPixel);
		m_textureBuffer[i] = new OculusTextureBuffer(m_hmdDevice, state, recommenedTextureSize);
		m_depthBuffer[i] = new OculusDepthBuffer(recommenedTextureSize, state);
	}
}

void OculusDevice::init() {
	initializeEyeRenderDesc();

	calculateEyeAdjustment();

	calculateProjectionMatrices();

	unsigned int hmdCaps = ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
	// Set render capabilities
	ovrHmd_SetEnabledCaps(m_hmdDevice, hmdCaps);

	// Start the sensor which provides the Rift's pose and motion.
	ovrHmd_ConfigureTracking(m_hmdDevice, ovrTrackingCap_Orientation |
		ovrTrackingCap_MagYawCorrection |
		ovrTrackingCap_Position, 0);

	// Setup layers
	setupLayers();
}

unsigned int OculusDevice::screenResolutionWidth() const
{
	return m_hmdDevice->Resolution.w;
}

unsigned int OculusDevice::screenResolutionHeight() const
{
	return m_hmdDevice->Resolution.h;
}

osg::Matrix OculusDevice::projectionMatrixCenter() const
{
	osg::Matrix projectionMatrixCenter;
	projectionMatrixCenter = m_leftEyeProjectionMatrix.operator*(0.5) + m_rightEyeProjectionMatrix.operator*(0.5);
	return projectionMatrixCenter;
}

osg::Matrix OculusDevice::projectionMatrixLeft() const
{
	return m_leftEyeProjectionMatrix;
}

osg::Matrix OculusDevice::projectionMatrixRight() const
{
	return m_rightEyeProjectionMatrix;
}

osg::Matrix OculusDevice::projectionOffsetMatrixLeft() const
{
	osg::Matrix projectionOffsetMatrix;
	float offset = m_leftEyeProjectionMatrix(2, 0);
	projectionOffsetMatrix.makeTranslate(osg::Vec3(-offset, 0.0, 0.0));
	return projectionOffsetMatrix;
}

osg::Matrix OculusDevice::projectionOffsetMatrixRight() const
{
	osg::Matrix projectionOffsetMatrix;
	float offset = m_rightEyeProjectionMatrix(2, 0);
	projectionOffsetMatrix.makeTranslate(osg::Vec3(-offset, 0.0, 0.0));
	return projectionOffsetMatrix;
}

osg::Matrix OculusDevice::viewMatrixLeft() const
{
	osg::Matrix viewMatrix;
	viewMatrix.makeTranslate(m_leftEyeAdjust);
	return viewMatrix;
}

osg::Matrix OculusDevice::viewMatrixRight() const
{
	osg::Matrix viewMatrix;
	viewMatrix.makeTranslate(m_rightEyeAdjust);
	return viewMatrix;
}

void OculusDevice::resetSensorOrientation() const {
	ovrHmd_RecenterPose(m_hmdDevice);
}

void OculusDevice::updatePose(unsigned int frameIndex)
{
	// Ask the API for the times when this frame is expected to be displayed.
	m_frameTiming = ovrHmd_GetFrameTiming(m_hmdDevice, frameIndex);

	m_viewOffset[0] = m_eyeRenderDesc[0].HmdToEyeViewOffset;
	m_viewOffset[1] = m_eyeRenderDesc[1].HmdToEyeViewOffset;
	
	// Query the HMD for the current tracking state.
	ovrTrackingState ts = ovrHmd_GetTrackingState(m_hmdDevice, m_frameTiming.DisplayMidpointSeconds);
	ovr_CalcEyePoses(ts.HeadPose.ThePose, m_viewOffset, m_eyeRenderPose);
	ovrPoseStatef headpose = ts.HeadPose;
	ovrPosef pose = headpose.ThePose;
	m_position.set(-pose.Position.x, -pose.Position.y, -pose.Position.z);
	m_position *= m_worldUnitsPerMetre;
	m_orientation.set(pose.Orientation.x, pose.Orientation.y, pose.Orientation.z, -pose.Orientation.w);
}

osg::Camera* OculusDevice::createRTTCamera(OculusDevice::Eye eye, osg::Transform::ReferenceFrame referenceFrame, osg::GraphicsContext* gc) const
{
	osg::Texture2D* texture = m_textureBuffer[renderOrder(eye)]->texture();
	osg::Texture2D* depth = m_depthBuffer[renderOrder(eye)]->texture();
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	camera->setRenderOrder(osg::Camera::PRE_RENDER, renderOrder(eye));
	camera->setCullingMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	camera->setAllowEventFocus(false);
	camera->setReferenceFrame(referenceFrame);

	camera->setGraphicsContext(gc);
	if (texture) {
		camera->setViewport(0, 0, texture->getTextureWidth(), texture->getTextureHeight());
		camera->attach(osg::Camera::COLOR_BUFFER, texture);
	}

	if (depth) {
		camera->attach(osg::Camera::DEPTH_BUFFER, depth);
	}

	camera->setPreDrawCallback(new OculusPreDrawCallback(camera.get(), m_textureBuffer[renderOrder(eye)], m_depthBuffer[renderOrder(eye)]));

	return camera.release();
}

bool OculusDevice::submitFrame(unsigned int frameIndex) {

	m_layerEyeFov.ColorTexture[0] = m_textureBuffer[0]->textureSet();
	m_layerEyeFov.ColorTexture[1] = m_textureBuffer[1]->textureSet();

	// Set render pose
	m_layerEyeFov.RenderPose[0] = m_eyeRenderPose[0];
	m_layerEyeFov.RenderPose[1] = m_eyeRenderPose[1];

	ovrLayerHeader* layers = &m_layerEyeFov.Header;
	ovrViewScaleDesc viewScale;
	viewScale.HmdToEyeViewOffset[0] = m_viewOffset[0];
	viewScale.HmdToEyeViewOffset[1] = m_viewOffset[1];
	viewScale.HmdSpaceToWorldScaleInMeters = m_worldUnitsPerMetre;
	ovrResult result = ovrHmd_SubmitFrame(m_hmdDevice, frameIndex, &viewScale, &layers, 1);
	return result == ovrSuccess;
}

void OculusDevice::toggleMirrorToWindow() {
	//unsigned int hmdCaps = ovrHmd_GetEnabledCaps(m_hmdDevice);
	//hmdCaps ^= ovrHmdCap_NoMirrorToWindow;
	//ovrHmd_SetEnabledCaps(m_hmdDevice, hmdCaps);
}

void OculusDevice::toggleLowPersistence() {
	unsigned int hmdCaps = ovrHmd_GetEnabledCaps(m_hmdDevice);
	hmdCaps ^= ovrHmdCap_LowPersistence;
	ovrHmd_SetEnabledCaps(m_hmdDevice, hmdCaps);
}

void OculusDevice::toggleDynamicPrediction() {
	unsigned int hmdCaps = ovrHmd_GetEnabledCaps(m_hmdDevice);
	hmdCaps ^= ovrHmdCap_DynamicPrediction;
	ovrHmd_SetEnabledCaps(m_hmdDevice, hmdCaps);
}

osg::GraphicsContext::Traits* OculusDevice::graphicsContextTraits() const {
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
	traits->windowDecoration = true;
	traits->x = 50;
	traits->y = 50;
	traits->width = screenResolutionWidth() / 2;
	traits->height = screenResolutionHeight() / 2;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->vsync = false; // VSync should always be disabled for Oculus Rift applications, the SDK compositor handles the swap

	return traits.release();
}

/* Protected functions */
OculusDevice::~OculusDevice()
{
	for (int i = 0; i < 2; i++) {
		ovrHmd_DestroySwapTextureSet(m_hmdDevice, m_textureBuffer[i]->textureSet());
	}
	ovrHmd_Destroy(m_hmdDevice);
	ovr_Shutdown();
}


int OculusDevice::renderOrder(Eye eye) const {
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

void OculusDevice::printHMDDebugInfo() {
	osg::notify(osg::ALWAYS) << "Product:         " << m_hmdDevice->ProductName << std::endl;
	osg::notify(osg::ALWAYS) << "Manufacturer:    " << m_hmdDevice->Manufacturer << std::endl;
	osg::notify(osg::ALWAYS) << "VendorId:        " << m_hmdDevice->VendorId << std::endl;
	osg::notify(osg::ALWAYS) << "ProductId:       " << m_hmdDevice->ProductId << std::endl;
	osg::notify(osg::ALWAYS) << "SerialNumber:    " << m_hmdDevice->SerialNumber << std::endl;
	osg::notify(osg::ALWAYS) << "FirmwareVersion: " << m_hmdDevice->FirmwareMajor << "." << m_hmdDevice->FirmwareMinor << std::endl;
}

void OculusDevice::initializeEyeRenderDesc() {
	m_eyeRenderDesc[0] = ovrHmd_GetRenderDesc(m_hmdDevice, ovrEye_Left, m_hmdDevice->DefaultEyeFov[0]);
	m_eyeRenderDesc[1] = ovrHmd_GetRenderDesc(m_hmdDevice, ovrEye_Right, m_hmdDevice->DefaultEyeFov[1]);
}

void OculusDevice::calculateEyeAdjustment() {
	ovrVector3f leftEyeAdjust = m_eyeRenderDesc[0].HmdToEyeViewOffset;
	m_leftEyeAdjust.set(leftEyeAdjust.x, leftEyeAdjust.y, leftEyeAdjust.z);
	m_leftEyeAdjust *= m_worldUnitsPerMetre;
	ovrVector3f rightEyeAdjust = m_eyeRenderDesc[1].HmdToEyeViewOffset;
	m_rightEyeAdjust.set(rightEyeAdjust.x, rightEyeAdjust.y, rightEyeAdjust.z);
	m_rightEyeAdjust *= m_worldUnitsPerMetre;
}

void OculusDevice::calculateProjectionMatrices() {
	bool isRightHanded = true;

	ovrMatrix4f leftEyeProjectionMatrix = ovrMatrix4f_Projection(m_eyeRenderDesc[0].Fov, m_nearClip, m_farClip, isRightHanded);
	// Transpose matrix
	m_leftEyeProjectionMatrix.set(leftEyeProjectionMatrix.M[0][0], leftEyeProjectionMatrix.M[1][0], leftEyeProjectionMatrix.M[2][0], leftEyeProjectionMatrix.M[3][0],
		leftEyeProjectionMatrix.M[0][1], leftEyeProjectionMatrix.M[1][1], leftEyeProjectionMatrix.M[2][1], leftEyeProjectionMatrix.M[3][1],
		leftEyeProjectionMatrix.M[0][2], leftEyeProjectionMatrix.M[1][2], leftEyeProjectionMatrix.M[2][2], leftEyeProjectionMatrix.M[3][2],
		leftEyeProjectionMatrix.M[0][3], leftEyeProjectionMatrix.M[1][3], leftEyeProjectionMatrix.M[2][3], leftEyeProjectionMatrix.M[3][3]);

	ovrMatrix4f rightEyeProjectionMatrix = ovrMatrix4f_Projection(m_eyeRenderDesc[1].Fov, m_nearClip, m_farClip, isRightHanded);
	// Transpose matrix
	m_rightEyeProjectionMatrix.set(rightEyeProjectionMatrix.M[0][0], rightEyeProjectionMatrix.M[1][0], rightEyeProjectionMatrix.M[2][0], rightEyeProjectionMatrix.M[3][0],
		rightEyeProjectionMatrix.M[0][1], rightEyeProjectionMatrix.M[1][1], rightEyeProjectionMatrix.M[2][1], rightEyeProjectionMatrix.M[3][1],
		rightEyeProjectionMatrix.M[0][2], rightEyeProjectionMatrix.M[1][2], rightEyeProjectionMatrix.M[2][2], rightEyeProjectionMatrix.M[3][2],
		rightEyeProjectionMatrix.M[0][3], rightEyeProjectionMatrix.M[1][3], rightEyeProjectionMatrix.M[2][3], rightEyeProjectionMatrix.M[3][3]);
}

void OculusDevice::setupLayers() {
	m_layerEyeFov.Header.Type = ovrLayerType_EyeFov;
	m_layerEyeFov.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

	ovrRecti viewPort[2];
	viewPort[0].Pos.x = 0;
	viewPort[0].Pos.y = 0;
	viewPort[0].Size.w = m_textureBuffer[0]->textureWidth();
	viewPort[0].Size.h = m_textureBuffer[0]->textureHeight();

	viewPort[1].Pos.x = 0;
	viewPort[1].Pos.y = 0;
	viewPort[1].Size.w = m_textureBuffer[1]->textureWidth();
	viewPort[1].Size.h = m_textureBuffer[1]->textureHeight();

	m_layerEyeFov.Viewport[0] = viewPort[0];
	m_layerEyeFov.Viewport[1] = viewPort[1];
	m_layerEyeFov.Fov[0] = m_eyeRenderDesc[0].Fov;
	m_layerEyeFov.Fov[1] = m_eyeRenderDesc[1].Fov;
}

void OculusDevice::trySetProcessAsHighPriority() const {
	// Require at least 4 processors, otherwise the process could occupy the machine.
	if (OpenThreads::GetNumberOfProcessors() >= 4) {
#ifdef _WIN32
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif
	}
}

void OculusRealizeOperation::operator() (osg::GraphicsContext* gc) {
	if (!m_realized) {
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
		gc->makeCurrent();

		if (osgViewer::GraphicsWindow* window = dynamic_cast<osgViewer::GraphicsWindow*>(gc)) {
			// Run wglSwapIntervalEXT(0) to force VSync Off
			window->setSyncToVBlank(false);
		}

		osg::ref_ptr<osg::State> state = gc->getState();
		m_device->createRenderBuffers(state);
		// Init the oculus system
		m_device->init();
	}
	m_realized = true;
}


void OculusSwapCallback::swapBuffersImplementation(osg::GraphicsContext *gc) {
	// Submit rendered frame to compositor
	m_device->submitFrame();
	// Run the default system swapBufferImplementation
	gc->swapBuffersImplementation();
}


