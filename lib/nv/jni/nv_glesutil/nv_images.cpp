//----------------------------------------------------------------------------------
// File:            libs\jni\nv_glesutil\nv_images.cpp
// Samples Version: Android NVIDIA samples 2 
// Email:           tegradev@nvidia.com
// Forum:           http://developer.nvidia.com/tegra/forums/tegra-forums/android-development
//
// Copyright 2009-2010 NVIDIA® Corporation 
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//----------------------------------------------------------------------------------


#include "../nv_global.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "nv_images.h"
#include "../nv_log/nv_log.h"

#include "../nv_file/nv_file.h"

static int LoadTextureFromDDSData(GLenum target, NvS32 startLevel, const NVHHDDSImage *pImage, NvBool useMipmaps )
{
    NvS32 w, h, level, m;
    NvS32 powerOfTwo, mipLevelsToLoad, maxLevel;
    GLenum baseTarget = (target == GL_TEXTURE_2D) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;

    if ( pImage == NULL ) {
        return 0;
    }

    w = pImage->width;
    h = pImage->height;
    level = startLevel;

    /* Need to detect if the image has a non-pow-2 dimension */
    powerOfTwo = ((w & (w - 1)) == 0) && ((h & (h - 1)) == 0);

    /* Only pow-2 images that are tagged for using mipmaps get them */
    mipLevelsToLoad = ((powerOfTwo && useMipmaps) ? pImage->numMipmaps : 1);

    maxLevel = level + mipLevelsToLoad;

    for ( m = 0; level < maxLevel; level++, m++ ) {
        if (pImage->compressed) {
            glCompressedTexImage2D( target, m, pImage->format, w, h,
                                    0, pImage->size[level],
                                    pImage->data[level] );
        } else {
            glTexImage2D( target, m, pImage->format, w, h, 0,
                          pImage->format, pImage->componentFormat,
                          pImage->data[level]);
        }

        w = (w == 1) ? w : w >> 1;
        h = (h == 1) ? h : h >> 1;
    }

    if (mipLevelsToLoad > 1)
    {
        glTexParameterf(baseTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    }
    else
    {
        glTexParameterf(baseTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    glTexParameterf(baseTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* For now, we clamp everyone - apps wanting wrapping for a texture can set it manually */
    glTexParameterf(baseTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(baseTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return 1;
}


GLuint NvCreateTextureFromDDSEx(const char* filename, NvBool flipVertical, NvBool useMipmaps, NvS32* width, NvS32* height, NvBool* alpha, NvBool *isCubeMap)
{
    // Clear the GL error before this function, since at the end of this 
    // function, we test the error and report it.  We do not want to report
    // an error that happened at some random time before.  If we want to
    // catch those, we need more general/comprehensive handling.  But having
    // this function print a GL error for something that happened in other 
    // random code is confusing, especially to non-rail developers.
    // Some other code, like NVBitfont, prints error messages at the top of
    // the function and print a message that implies that the error was there
    // at the time of call.  That may make sense as an optional setting down
    // the road
    glGetError();

    GLuint tex = 0;
    NVHHDDSImage *img = NVHHDDSLoad(filename, flipVertical ? 1 : 0);
    if (img)
    {
        if(isCubeMap)
        {
            *isCubeMap = img->cubemap ? NV_TRUE:NV_FALSE;
        }
        if (width)
            *width = img->width;
        if (height)
            *height = img->height;
        if (alpha)
            *alpha = img->alpha ? NV_TRUE : NV_FALSE;

        glGenTextures(1, &tex);
        if (!img->cubemap)
        {
            glBindTexture(GL_TEXTURE_2D, tex);
            LoadTextureFromDDSData(GL_TEXTURE_2D, 0, img, useMipmaps);
        }
        else
        {
            NvS32 baseLevel = 0;
            glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

            LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, baseLevel, img, useMipmaps);
            baseLevel += img->numMipmaps ? img->numMipmaps : 1;

            LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_POSITIVE_X, baseLevel, img, useMipmaps);
            baseLevel += img->numMipmaps ? img->numMipmaps : 1;

            LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, baseLevel, img, useMipmaps);
            baseLevel += img->numMipmaps ? img->numMipmaps : 1;

            LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, baseLevel, img, useMipmaps);
            baseLevel += img->numMipmaps ? img->numMipmaps : 1;

            LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, baseLevel, img, useMipmaps);
            baseLevel += img->numMipmaps ? img->numMipmaps : 1;

            LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, baseLevel, img, useMipmaps);
            baseLevel += img->numMipmaps ? img->numMipmaps : 1;
        }

        GLint err = glGetError();
        if (err)
            NVLogError("NvCreateTextureFromDDSEx error", "");

        NVHHDDSFree(img);
    }

    return tex;
}

GLuint NvCreateTextureFromHHDDSImage(const NVHHDDSImage& image,
                                NvBool useMipmaps, NvS32* width, NvS32* height, NvBool* alpha, NvBool *isCubeMap)
{
    // Clear the GL error before this function, since at the end of this 
    // function, we test the error and report it.  We do not want to report
    // an error that happened at some random time before.  If we want to
    // catch those, we need more general/comprehensive handling.  But having
    // this function print a GL error for something that happened in other 
    // random code is confusing, especially to non-rail developers.
    // Some other code, like NVBitfont, prints error messages at the top of
    // the function and print a message that implies that the error was there
    // at the time of call.  That may make sense as an optional setting down
    // the road
    glGetError();

    GLuint tex = 0;
    if(isCubeMap)
    {
        *isCubeMap = image.cubemap ? NV_TRUE:NV_FALSE;
    }
    if (width)
        *width = image.width;
    if (height)
        *height = image.height;
    if (alpha)
        *alpha = image.alpha ? NV_TRUE : NV_FALSE;

    glGenTextures(1, &tex);
    if (!image.cubemap)
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        LoadTextureFromDDSData(GL_TEXTURE_2D, 0, &image, useMipmaps);
    }
    else
    {
        NvS32 baseLevel = 0;
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

        LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, baseLevel, &image, useMipmaps);
        baseLevel += image.numMipmaps ? image.numMipmaps : 1;

        LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_POSITIVE_X, baseLevel, &image, useMipmaps);
        baseLevel += image.numMipmaps ? image.numMipmaps : 1;

        LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, baseLevel, &image, useMipmaps);
        baseLevel += image.numMipmaps ? image.numMipmaps : 1;

        LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, baseLevel, &image, useMipmaps);
        baseLevel += image.numMipmaps ? image.numMipmaps : 1;

        LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, baseLevel, &image, useMipmaps);
        baseLevel += image.numMipmaps ? image.numMipmaps : 1;

        LoadTextureFromDDSData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, baseLevel, &image, useMipmaps);
        baseLevel += image.numMipmaps ? image.numMipmaps : 1;
    }

    GLint err = glGetError();
    if (err)
        NVLogError("NvCreateTextureFromHHDDSImage error", "");

    return tex;
}

