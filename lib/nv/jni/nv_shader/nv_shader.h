//----------------------------------------------------------------------------------
// File:            libs\jni\nv_shader\nv_shader.h
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

#ifndef __INCLUDED_NV_SHADER_H
#define __INCLUDED_NV_SHADER_H

#include <GLES2/gl2.h>
#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>

/** @file nv_shader.h
  The nv_shader library includes functions to make it easy to load, compile
  and link shader programs in a bound OpenGL ES 2.0 context.  Uses nv_file as
  a part of its implementation.
  @see nv_file.h
  */

/**
  Initializes the library.  This function MUST be called from the application's
  JNI_OnLoad, from a function known to be called by JNI_OnLoad, or from a function
  in a Java-called thread.  nv_thread-created native threads cannot call this
  initialization function.  Calls NvFInit() internally.
  @see nv_file.h
  @see nv_thread.h
  */
void nv_shader_init();

/**
  Loads a pair of shaders from the given path, using NvFile to find the files.
  Loads .vert for the vertex shader and .frag for the fragment shader.
  @param prog The name (and path) of the shader files, with no extension specified
  @param prepend Optional string to be prepended to the source in both shader files.
  @return nonzero shader program ID in the currently-bound context on success,
  and zero on failure
  */
GLint nv_load_program(const char *prog, const char *prepend = NULL);

/**
  Finds the given named vertex attribute for the specified program and sets up
  the vertex array for that attrbiute.
  @param prog The program to be queried for index
  @param name The string name of the attribute
  @param size Same use as in the function glVertexAttribPointer
  @param type Same use as in the function glVertexAttribPointer
  @param normalized Same use as in the function glVertexAttribPointer
  @param stride Same use as in the function glVertexAttribPointer
  @param ptr Same use as in the function glVertexAttribPointer
  */
void nv_set_attrib_by_name(GLuint prog, const char *name, int size, GLenum type,
        GLboolean normalized, GLsizei stride, void *ptr);

/**
  Sets up a vertex array for a specified attrbiute
  @param index The index identifying the attribute
  @param size Same use as in the function glVertexAttribPointer
  @param type Same use as in the function glVertexAttribPointer
  @param normalized Same use as in the function glVertexAttribPointer
  @param stride Same use as in the function glVertexAttribPointer
  @param ptr Same use as in the function glVertexAttribPointer
  */
void nv_set_attrib_by_index(GLint index, int size, GLenum type,
        GLboolean normalized, GLsizei stride, void *ptr);

/**
  Disables all vertex attributes.  <b>Currently unimplemented</b>
  */
void nv_flush_tracked_attribs();


#endif
