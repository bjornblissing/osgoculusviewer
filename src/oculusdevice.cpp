/*
 * oculusdevice.cpp
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 *
 */

#include "oculusdevice.h"

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <osgViewer/Renderer>
#include <osgViewer/GraphicsWindow>

OculusDevice::OculusDevice(float nearClip, float farClip, const float pixelsPerDisplayPixel, const float worldUnitsPerMetre, const int samples, unsigned int mirrorTextureWidth) :
	m_session(nullptr),
	m_hmdDesc(),
	m_pixelsPerDisplayPixel(pixelsPerDisplayPixel),
	m_worldUnitsPerMetre(worldUnitsPerMetre),
	m_mirrorTexture(nullptr),
   m_mirrorTextureWidth(mirrorTextureWidth),
	m_position(osg::Vec3(0.0f, 0.0f, 0.0f)),
	m_orientation(osg::Quat(0.0f, 0.0f, 0.0f, 1.0f)),
	m_nearClip(nearClip), m_farClip(farClip),
	m_samples(samples)
{
	for (int i = 0; i < 2; i++)
	{
		m_textureBuffer[i] = nullptr;
	}

	trySetProcessAsHighPriority();
	
	ovrResult result = ovr_Initialize(nullptr);

	if (result != ovrSuccess)
	{
		osg::notify(osg::WARN) << "Warning: Unable to initialize the Oculus library!" << std::endl;
		return;
	}

	ovrGraphicsLuid luid;

	// Get first available HMD
	result = ovr_Create(&m_session, &luid);

	if (result != ovrSuccess)
	{
		osg::notify(osg::WARN) << "Warning: No device could be found." << std::endl;
		return;
	}

	// Get HMD description
	m_hmdDesc = ovr_GetHmdDesc(m_session);

	// Print information about device
	printHMDDebugInfo();
}

void OculusDevice::createRenderBuffers(osg::ref_ptr<osg::State> state)
{
	// Compute recommended render texture size
	if (m_pixelsPerDisplayPixel > 1.0f)
	{
		osg::notify(osg::WARN) << "Warning: Pixel per display pixel is set to a value higher than 1.0." << std::endl;
	}

	for (int i = 0; i < 2; i++)
	{
		ovrSizei recommenedTextureSize = ovr_GetFovTextureSize(m_session, (ovrEyeType)i, m_hmdDesc.DefaultEyeFov[i], m_pixelsPerDisplayPixel);
		m_textureBuffer[i] = new OculusTextureBuffer(m_session, state, recommenedTextureSize, m_samples);
	}

	// compute mirror texture height based on requested with and respecting the Oculus screen ar
	int height = (float)m_mirrorTextureWidth / (float)screenResolutionWidth() * (float)screenResolutionHeight();
	m_mirrorTexture = new OculusMirrorTexture(m_session, state, m_mirrorTextureWidth, height);
}

void OculusDevice::init()
{
	getEyeRenderDesc();

	setupLayers();

	// Reset perf hud
	ovr_SetInt(m_session, "PerfHudMode", (int)ovrPerfHud_Off);
}

void OculusDevice::destroyTextures(osg::GraphicsContext* gc) 
{
	// Delete mirror texture
	if (m_mirrorTexture.valid())
	{
		m_mirrorTexture->destroy(gc);
	}

	// Delete texture and depth buffers
	for (int i = 0; i < 2; i++)
	{
		if (m_textureBuffer[i].valid())
		{
			m_textureBuffer[i]->destroy();
		}
	}
}

bool OculusDevice::hmdPresent() const
{
	ovrSessionStatus status;

	if (m_session)
	{
		ovrResult result = ovr_GetSessionStatus(m_session, &status);

		if (result == ovrSuccess)
		{
			return (status.HmdPresent == ovrTrue);
		}
		else
		{
			ovrErrorInfo error;
			ovr_GetLastErrorInfo(&error);
			osg::notify(osg::WARN) << error.ErrorString << std::endl;
			return false;
		}
	}

	return false;
}

void OculusDevice::updatePose(long long frameIndex) 
{
	// Call getEyeRenderDesc() each frame, since the returned values (e.g. HmdToEyePose) may change at runtime.
	getEyeRenderDesc();

	ovrPosef HmdToEyePose[2] = { m_eyeRenderDesc[0].HmdToEyePose, m_eyeRenderDesc[1].HmdToEyePose };
	ovr_GetEyePoses(m_session, frameIndex, ovrTrue, HmdToEyePose, m_eyeRenderPose, &m_sensorSampleTime);
}

osg::Vec3 OculusDevice::position(Eye eye) const {
	return osg::Vec3(m_eyeRenderPose[eye].Position.x, m_eyeRenderPose[eye].Position.y, m_eyeRenderPose[eye].Position.z);
}

osg::Quat OculusDevice::orientation(Eye eye) const {
	return osg::Quat(m_eyeRenderPose[eye].Orientation.x, m_eyeRenderPose[eye].Orientation.y, m_eyeRenderPose[eye].Orientation.z, m_eyeRenderPose[eye].Orientation.w);
}

osg::Matrixf OculusDevice::viewMatrix(Eye eye) const
{
	osg::Matrix viewMatrix;
	ovrPosef pose = m_eyeRenderDesc[eye].HmdToEyePose;

	osg::Vec3 position(pose.Position.x, pose.Position.y, pose.Position.z);
	osg::Quat orientation(pose.Orientation.x, pose.Orientation.y, pose.Orientation.z, pose.Orientation.w);

	viewMatrix.setTrans(position);
	viewMatrix.setRotate(orientation);

	// Scale to world units
	viewMatrix.postMultScale(osg::Vec3d(m_worldUnitsPerMetre, m_worldUnitsPerMetre, m_worldUnitsPerMetre));

	return viewMatrix;
}

osg::Matrixf OculusDevice::projectionMatrix(Eye eye) const
{
	osg::Matrix projectionMatrix;
	ovrMatrix4f ovrProjectionMatrix = ovrMatrix4f_Projection(m_eyeRenderDesc[eye].Fov, m_nearClip, m_farClip, ovrProjection_ClipRangeOpenGL);
	// Transpose matrix
	projectionMatrix.set(ovrProjectionMatrix.M[0][0], ovrProjectionMatrix.M[1][0], ovrProjectionMatrix.M[2][0], ovrProjectionMatrix.M[3][0],
		ovrProjectionMatrix.M[0][1], ovrProjectionMatrix.M[1][1], ovrProjectionMatrix.M[2][1], ovrProjectionMatrix.M[3][1],
		ovrProjectionMatrix.M[0][2], ovrProjectionMatrix.M[1][2], ovrProjectionMatrix.M[2][2], ovrProjectionMatrix.M[3][2],
		ovrProjectionMatrix.M[0][3], ovrProjectionMatrix.M[1][3], ovrProjectionMatrix.M[2][3], ovrProjectionMatrix.M[3][3]);

	return projectionMatrix;
}

osg::Camera* OculusDevice::createRTTCamera(OculusDevice::Eye eye, osg::Transform::ReferenceFrame referenceFrame, const osg::Vec4& clearColor, osg::GraphicsContext* gc) const
{
	osg::ref_ptr<OculusTextureBuffer> buffer = m_textureBuffer[eye];

	osg::ref_ptr<osg::Camera> camera = new osg::Camera();
	camera->setClearColor(clearColor);
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	camera->setRenderOrder(osg::Camera::PRE_RENDER, eye);
	camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	camera->setAllowEventFocus(false);
	camera->setReferenceFrame(referenceFrame);
	camera->setViewport(0, 0, buffer->textureWidth(), buffer->textureHeight());
	camera->setGraphicsContext(gc);

	if (buffer->colorBuffer())
	{
		camera->attach(osg::Camera::COLOR_BUFFER, buffer->colorBuffer().get());
	}

	if (buffer->depthBuffer())
	{
		camera->attach(osg::Camera::DEPTH_BUFFER, buffer->depthBuffer().get());
	}

	if (m_samples != 0)
	{
		// If we are using MSAA, we don't want OSG doing anything regarding FBO
		// setup and selection because this is handled completely by 'setupMSAA'
		// and by pre and post render callbacks. So this initial draw callback is 
		// used to disable normal OSG camera setup which would undo the MSAA buffer
		// configuration. Note that we have also implicitly avoided the camera buffer 
		// attachments above when MSAA is enabled because we don't want OSG to 
		// affect the texture bindings handled by the pre and post render callbacks.
		camera->setInitialDrawCallback(new OculusInitialDrawCallback());
	}

	camera->setPreDrawCallback(new OculusPreDrawCallback(camera.get(), buffer.get()));
	camera->setFinalDrawCallback(new OculusPostDrawCallback(camera.get(), buffer.get()));

	return camera.release();
}

bool OculusDevice::waitToBeginFrame(long long frameIndex)
{
	ovrResult error = ovr_WaitToBeginFrame(m_session, frameIndex);
	return (error == ovrSuccess);
}

bool OculusDevice::beginFrame(long long frameIndex)
{
	m_sensorSampleTime = ovr_GetPredictedDisplayTime(m_session, frameIndex);

	ovrResult error = ovr_BeginFrame(m_session, frameIndex);
	return (error == ovrSuccess);
}

bool OculusDevice::submitFrame(long long frameIndex)
{
	m_layerEyeFov.ColorTexture[0] = m_textureBuffer[0]->textureSwapChain();
	m_layerEyeFov.ColorTexture[1] = m_textureBuffer[1]->textureSwapChain();

	m_layerEyeFov.Fov[0] = m_eyeRenderDesc[0].Fov;
	m_layerEyeFov.Fov[1] = m_eyeRenderDesc[1].Fov;

	m_layerEyeFov.RenderPose[0] = m_eyeRenderPose[0];
	m_layerEyeFov.RenderPose[1] = m_eyeRenderPose[1];

	m_layerEyeFov.SensorSampleTime = m_sensorSampleTime;

	ovrLayerHeader* layers = &m_layerEyeFov.Header;
	ovrViewScaleDesc viewScale;
	viewScale.HmdToEyePose[0] = m_eyeRenderDesc[0].HmdToEyePose;
	viewScale.HmdToEyePose[1] = m_eyeRenderDesc[1].HmdToEyePose;
	viewScale.HmdSpaceToWorldScaleInMeters = m_worldUnitsPerMetre;
	ovrResult result = ovr_EndFrame(m_session, frameIndex, &viewScale, &layers, 1);
	return result == ovrSuccess;
}

void OculusDevice::blitMirrorTexture(osg::GraphicsContext* gc)
{
	m_mirrorTexture->blitTexture(gc);
}

void OculusDevice::setPerfHudMode(int mode)
{
	if (mode == 0) { ovr_SetInt(m_session, "PerfHudMode", (int)ovrPerfHud_Off); }

	if (mode == 1) { ovr_SetInt(m_session, "PerfHudMode", (int)ovrPerfHud_PerfSummary); }

	if (mode == 2) { ovr_SetInt(m_session, "PerfHudMode", (int)ovrPerfHud_LatencyTiming); }

	if (mode == 3) { ovr_SetInt(m_session, "PerfHudMode", (int)ovrPerfHud_AppRenderTiming); }

	if (mode == 4) { ovr_SetInt(m_session, "PerfHudMode", (int)ovrPerfHud_CompRenderTiming); }
	
	if (mode == 5) { ovr_SetInt(m_session, "PerfHudMode", (int)ovrPerfHud_VersionInfo); }
}

osg::GraphicsContext::Traits* OculusDevice::graphicsContextTraits() const
{
	// Create screen with match the Oculus Rift resolution
	osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();

	if (!wsi)
	{
		osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
		return 0;
	}

	// Get the screen identifiers set in environment variable DISPLAY
	osg::GraphicsContext::ScreenIdentifier si;
	si.readDISPLAY();

	// If displayNum has not been set, reset it to 0.
	if (si.displayNum < 0)
	{
		si.displayNum = 0;
		osg::notify(osg::INFO) << "Couldn't get display number, setting to 0" << std::endl;
	}

	// If screenNum has not been set, reset it to 0.
	if (si.screenNum < 0)
	{
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
	traits->width = m_mirrorTextureWidth;
	traits->height = (float)m_mirrorTextureWidth / (float)screenResolutionWidth() * (float)screenResolutionHeight();
	traits->doubleBuffer = true;
	traits->sharedContext = nullptr;
	traits->vsync = false; // VSync should always be disabled for Oculus Rift applications, the SDK compositor handles the swap

	return traits.release();
}

/* Protected functions */
OculusDevice::~OculusDevice()
{
	// Destroy the textures if before OSG 3.5.4
	// For version 3.5.4 or later this should be handled 
	// by a CleanUpOperation set in the viewer (see the OculusViewerExample)
#if(OSG_VERSION_LESS_THAN(3, 5, 4))
	destroyTextures(nullptr);
#endif

	ovr_Destroy(m_session);
	ovr_Shutdown();
}

void OculusDevice::printHMDDebugInfo()
{
	osg::notify(osg::ALWAYS) << "Product:         " << m_hmdDesc.ProductName << std::endl;
	osg::notify(osg::ALWAYS) << "Manufacturer:    " << m_hmdDesc.Manufacturer << std::endl;
	osg::notify(osg::ALWAYS) << "VendorId:        " << m_hmdDesc.VendorId << std::endl;
	osg::notify(osg::ALWAYS) << "ProductId:       " << m_hmdDesc.ProductId << std::endl;
	osg::notify(osg::ALWAYS) << "SerialNumber:    " << m_hmdDesc.SerialNumber << std::endl;
	osg::notify(osg::ALWAYS) << "FirmwareVersion: " << m_hmdDesc.FirmwareMajor << "." << m_hmdDesc.FirmwareMinor << std::endl;
}

void OculusDevice::getEyeRenderDesc()
{
	m_eyeRenderDesc[0] = ovr_GetRenderDesc(m_session, ovrEye_Left, m_hmdDesc.DefaultEyeFov[0]);
	m_eyeRenderDesc[1] = ovr_GetRenderDesc(m_session, ovrEye_Right, m_hmdDesc.DefaultEyeFov[1]);
}

void OculusDevice::setupLayers()
{
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

void OculusDevice::trySetProcessAsHighPriority() const
{
	// Require at least 4 processors, otherwise the process could occupy the machine.
	if (OpenThreads::GetNumberOfProcessors() >= 4)
	{
#ifdef _WIN32
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif
	}
}

void OculusRealizeOperation::operator() (osg::GraphicsContext* gc)
{
	if (!m_realized)
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
		gc->makeCurrent();

		if (osgViewer::GraphicsWindow* window = dynamic_cast<osgViewer::GraphicsWindow*>(gc))
		{
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

void OculusCleanUpOperation::operator() (osg::GraphicsContext* gc)
{
	gc->makeCurrent();
	m_device->destroyTextures(gc);
}

void OculusSwapCallback::swapBuffersImplementation(osg::GraphicsContext* gc)
{
	// Submit rendered frame to compositor
	m_device->submitFrame(m_frameIndex++);

	// Blit mirror texture to backbuffer
	m_device->blitMirrorTexture(gc);

	// Run the default system swapBufferImplementation
	gc->swapBuffersImplementation();
}

void OculusInitialDrawCallback::operator()(osg::RenderInfo& renderInfo) const
{
	osg::GraphicsOperation* graphicsOperation = renderInfo.getCurrentCamera()->getRenderer();
	osgViewer::Renderer* renderer = dynamic_cast<osgViewer::Renderer*>(graphicsOperation);
	if (renderer != nullptr)
	{
		// Disable normal OSG FBO camera setup because it will undo the MSAA FBO configuration.
		renderer->setCameraRequiresSetUp(false);
	}
}
