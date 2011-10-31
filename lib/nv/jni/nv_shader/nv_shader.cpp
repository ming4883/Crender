//----------------------------------------------------------------------------------
// File:            libs\jni\nv_shader\nv_shader.cpp
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

#include "nv_shader.h"
#include "../nv_file/nv_file.h"

void nv_shader_init()
{
    NvFInit();
}

char* load_file(const char* file)
{
    NvFile *f = NvFOpen(file);
    size_t size = NvFSize(f);
    char *buffer = new char[size+1];
    NvFRead(buffer, 1, size, f);
    NvFClose(f);
    buffer[size] = '\0';

    return buffer;
}

static int glsl_log(GLuint obj, GLenum check_compile)
{
    if (check_compile == GL_COMPILE_STATUS)
    {
        int len = 0;
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
        if(len > 0)
        {
            char *str = (char *) malloc(len * sizeof(char));
            if (str)
            {
                glGetShaderInfoLog(obj, len, NULL, str);
                __android_log_print(ANDROID_LOG_DEBUG, "nv_shader",  "shader_debug: %s\n", str);
                free(str);
            }
        }
    }
    else
    {
        int len = 0;
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
        if(len > 0)
        {
            char *str = (char *)malloc(len * sizeof(char));
            if (str)
            {
                glGetProgramInfoLog(obj, len, NULL, str);
                __android_log_print(ANDROID_LOG_DEBUG, "nv_shader",  "shader_debug: %s\n", str);
                free(str);
            }
        }
    }
    return 0;
}

GLint nv_load_program(const char *prog, const char *prepend)
{
	int count = 0;
    char* shaders[2];
    int sizes[2];
    char filename[256];

    __android_log_print(ANDROID_LOG_DEBUG, "nv_shader", "in nv_load_program");
    __android_log_print(ANDROID_LOG_DEBUG, "nv_shader", "gl version: %s", glGetString(GL_VERSION));
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);

    if(prepend)
    {
		__android_log_print(ANDROID_LOG_DEBUG, "nv_shader",  "compiling string:\n\"%s\"\n", prepend);
		shaders[count] = (char*)prepend;
		sizes[count] = strlen(shaders[count]);
		count++;
    }

    if(prog)
    {
		sprintf(filename, "%s.vert", prog);
		__android_log_print(ANDROID_LOG_DEBUG, "nv_shader",  "compiling file: %s\n", filename);
		shaders[count] = load_file(filename);
		sizes[count] = strlen(shaders[count]);
		count++;
    }

    if(count == 0)
    	return (GLint)0;

    glShaderSource(vert, count, (const char **) shaders, sizes);
    count--;
    glCompileShader(vert);
    delete[] shaders[count];
    glsl_log(vert, GL_COMPILE_STATUS);

    sprintf(filename, "%s.frag", prog);
    __android_log_print(ANDROID_LOG_DEBUG, "nv_shader",  "compiling file: %s\n", filename);
    shaders[count] = load_file(filename);
    sizes[count] = strlen(shaders[count]);
    count++;

    glShaderSource(frag, count, (const char **) shaders, sizes);
    count--;
    glCompileShader(frag);
    delete[] shaders[count];
    glsl_log(frag, GL_COMPILE_STATUS);

    glLinkProgram(program);
    glsl_log(program, GL_LINK_STATUS);

    return program;
}


static void track_attrib_location(int index)
{
}

void nv_set_attrib_by_name(GLuint prog, const char *name, int size, GLenum type,
        GLboolean normalized, GLsizei stride, void *ptr)
 {
    int index = glGetAttribLocation(prog, name);
    glVertexAttribPointer(index, size, type, normalized, stride, ptr);
    glEnableVertexAttribArray(index);
    track_attrib_location(index);
}

void nv_set_attrib_by_index(GLint index, int size, GLenum type,
        GLboolean normalized, GLsizei stride, void *ptr)
 {
    glVertexAttribPointer(index, size, type, normalized, stride, ptr);
    glEnableVertexAttribArray(index);
    track_attrib_location(index);
}

void nv_flush_tracked_attribs()
{
}


