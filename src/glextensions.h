/*
 * glextensions.h
 *
 *  Created on: Dec 13, 2020
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_GLEXTENSIONS_H_
#define _OSG_GLEXTENSIONS_H_

#include <osg/GLExtensions>
#include <osg/Version>

#ifndef GL_TEXTURE_MAX_LEVEL
  #define GL_TEXTURE_MAX_LEVEL 0x813D
#endif

#if (OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
typedef osg::GLExtensions OSG_GLExtensions;
typedef osg::GLExtensions OSG_Texture_Extensions;
#else
typedef osg::FBOExtensions OSG_GLExtensions;
typedef osg::Texture::Extensions OSG_Texture_Extensions;
#endif

inline const OSG_GLExtensions* getGLExtensions(const osg::State& state) {
#if (OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
  return state.get<osg::GLExtensions>();
#else
  return osg::FBOExtensions::instance(state.getContextID(), true);
#endif
}
inline const OSG_Texture_Extensions* getTextureExtensions(const osg::State& state) {
#if (OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
  return state.get<osg::GLExtensions>();
#else
  return osg::Texture::getExtensions(state.getContextID(), true);
#endif
}

#endif /* _OSG_GLEXTENSIONS_H_ */
