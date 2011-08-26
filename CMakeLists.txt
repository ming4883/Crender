CMAKE_MINIMUM_REQUIRED( VERSION 2.8.4 )

PROJECT( crender )

SET( CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/dist" CACHE PATH "Use DESTDIR *instead* of CMAKE_INSTALL_PREFIX" FORCE ) 

FILE( GLOB GLEW lib/glew/*.c lib/glew/*.h )
FILE( GLOB GLSW lib/glsw/*.c lib/glsw/*.h )
FILE( GLOB HTTPD lib/httpd/*.c lib/httpd/*.h )

ADD_DEFINITIONS( -DGLEW_STATIC )

SET( CRENDER_COMMON
		lib/crender/Context.h
		lib/crender/Buffer.h
		lib/crender/GpuFormat.h
#		lib/crender/GpuState.h
		lib/crender/Mat44.c
		lib/crender/Mat44.h
		lib/crender/Mem.c
		lib/crender/Mem.h
		lib/crender/NvpParser.c
		lib/crender/NvpParser.h
		lib/crender/Platform.c
		lib/crender/Platform.h
		lib/crender/pstdint.h
		lib/crender/RenderTarget.h
		lib/crender/Shader.h
		lib/crender/StrHash.c
		lib/crender/StrHash.h
		lib/crender/StrHashMacro.h
		lib/crender/StrUtil.h
		lib/crender/Texture.h
		lib/crender/Vec2.c
		lib/crender/Vec2.h
		lib/crender/Vec3.c
		lib/crender/Vec3.h
		lib/crender/Vec4.c
		lib/crender/Vec4.h
		)

IF( WIN32 )

	#FILE( GLOB CRENDER lib/crender/*.c lib/crender/*.h )
	SET( CRENDER
		${CRENDER_COMMON}
		lib/crender/Context.windows.gl.c
		lib/crender/Context.gl.c
		lib/crender/Context.gl.h
		lib/crender/Buffer.gl.c
		lib/crender/Buffer.gl.h
#		lib/crender/GpuState.gl.c
#		lib/crender/GpuState.gl.h
		lib/crender/Mat44.gl.c
		lib/crender/RenderTarget.gl.c
		lib/crender/RenderTarget.gl.h
		lib/crender/Shader.gl.c
		lib/crender/Shader.gl.h
		lib/crender/Texture.gl.c
		lib/crender/Texture.gl.h
		)
	
	SET( CONSOLE_SYSTEM WIN32 )
	
	ADD_DEFINITIONS( -DCR_WIN32 )

	# assume MSVC
	ADD_DEFINITIONS( /wd4996 )
	SOURCE_GROUP( "glew" FILES ${GLEW} )
	SOURCE_GROUP( "glsw" FILES ${GLSW} )
	SOURCE_GROUP( "httpd" FILES ${HTTPD} )
	SOURCE_GROUP( "crender" FILES ${CRENDER} )
	SET( PLATFORM_LIBS opengl32 glu32 winmm ws2_32 )
	
	# d3d9 only on WIN32
	SET( PLATFORM_LIBS_D3D9 d3d9 d3dx9 winmm ws2_32 )

	SET( CRENDER_D3D9
		${CRENDER_COMMON}
		lib/crender/Context.windows.d3d9.c
		lib/crender/Context.d3d9.h
		lib/crender/Buffer.d3d9.c
		lib/crender/Buffer.d3d9.h
#		lib/crender/GpuState.d3d9.c
#		lib/crender/GpuState.d3d9.h
		lib/crender/Mat44.d3d9.c
		lib/crender/RenderTarget.d3d9.c
		lib/crender/RenderTarget.d3d9.h
		lib/crender/Shader.d3d9.c
		lib/crender/Shader.d3d9.h
		lib/crender/Texture.d3d9.c
		lib/crender/Texture.d3d9.h
		)
	SOURCE_GROUP( "crender" FILES ${CRENDER_D3D9} )
	
	ADD_LIBRARY( crender-d3d9
		${GLSW}
		${HTTPD}
		${CRENDER_D3D9}
		)
	
ELSEIF( APPLE )

	SET( CRENDER
		${CRENDER_COMMON}
		lib/crender/Context.ios.m
		lib/crender/Context.gl.c
		lib/crender/Context.gl.h
		lib/crender/Buffer.gl.c
		lib/crender/Buffer.gl.h
#		lib/crender/GpuState.gl.c
#		lib/crender/GpuState.gl.h
		lib/crender/Mat44.gl.c
		lib/crender/RenderTarget.gl.c
		lib/crender/RenderTarget.gl.h
		lib/crender/Shader.gl.c
		lib/crender/Shader.gl.h
		lib/crender/Texture.gl.c
		lib/crender/Texture.gl.h
		)
		
	SET( GLEW )
	
	SOURCE_GROUP( "glsw" FILES ${GLSW} )
	SOURCE_GROUP( "httpd" FILES ${HTTPD} )
	SOURCE_GROUP( "crender" FILES ${CRENDER} )	
		
	ADD_DEFINITIONS( -DCR_APPLE_IOS )
	ADD_DEFINITIONS( -DTARGET_OS_IPHONE )
	
	set(APP_TYPE MACOSX_BUNDLE)
	
	SET( CMAKE_OSX_SYSROOT iphoneos )
	SET( CMAKE_OSX_ARCHITECTURES $(ARCHS_STANDARD_32_BIT) )
	SET( CMAKE_CXX_FLAGS "-x objective-c++" )
	SET( CMAKE_EXE_LINKER_FLAGS
		"-framework Foundation -framework CoreGraphics -framework QuartzCore -framework UIKit -framework OpenGLES"
		)
	
	SET(MACOSX_BUNDLE_GUI_IDENTIFIER "com.crender.\${PRODUCT_NAME:identifier}")
	SET( CONSOLE_SYSTEM MACOSX_BUNDLE )
	
ELSEIF( UNIX )

	FILE( GLOB CRENDER crender/*.c crender/*.h )
	
	SET( PLATFORM_LIBS GL GLU X11 )
	
	ADD_DEFINITIONS( -DCR_LINUX )

ENDIF()

INCLUDE_DIRECTORIES(
	lib/glew
	lib/glsw
	lib/httpd
)

LIST( SORT CRENDER )
ADD_LIBRARY( crender
	${GLEW}
	${GLSW}
	${HTTPD}
	${CRENDER}
	)

# unit testes
IF( NOT APPLE )
	ADD_EXECUTABLE( test_crender
		test/TestXPRender.c
		lib/cutest/CuTest.c
		lib/cutest/CuTest.h
		lib/cutest/README.txt
		)

	TARGET_LINK_LIBRARIES( test_crender crender ${PLATFORM_LIBS} )

ENDIF()

# examples
ADD_SUBDIRECTORY( example )
