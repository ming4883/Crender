
SET( EXAMPLE_COMMON 
	Common.c
	Common.h
	Stream.c
	Stream.h
	Remote.c
	Remote.h
	Remote.impl.h
	Mesh.c
	Mesh.obj.c
	Mesh.h
	Material.c
	Material.h
	Pvr.c
	Pvr.h
	)

IF( WIN32 )
	SET( EXAMPLE_COMMON_D3D9
		${EXAMPLE_COMMON}
		../lib/crender/Framework.h
		../lib/crender/Framework.windows.d3d9.c
		Common.generic.c
		Remote.impl.windows.c
		Remote.impl.windows.h
	)
	
	LIST( APPEND EXAMPLE_COMMON
		../lib/crender/Framework.h
		../lib/crender/Framework.windows.gl.c
		Common.generic.c
		Remote.impl.windows.c
		Remote.impl.windows.h
	)
	
ELSEIF( APPLE )
	LIST( APPEND EXAMPLE_COMMON
		../lib/crender/Framework.h
		../lib/crender/Framework.ios.m
		Common.ios.m
		Remote.impl.ios.m
		Remote.impl.ios.h
	)
	
	SET( PLIST_FILE ${CMAKE_CURRENT_LIST_DIR}/Example-Info.plist )
	
ENDIF()

LIST( SORT EXAMPLE_COMMON )

SOURCE_GROUP( "common" FILES ${EXAMPLE_COMMON} )

MACRO( CREATE_EXAMPLE APP_NAME SRC RES )

IF( APPLE )
	
	SET( RES_GLES ${RES} )
	STRING( REPLACE glsl gles RES_GLES "${RES_GLES}" )
		
	ADD_EXECUTABLE( ${APP_NAME} ${CONSOLE_SYSTEM} ${SRC} ${RES_GLES} ${EXAMPLE_COMMON} )
	TARGET_LINK_LIBRARIES( ${APP_NAME} crender ${PLATFORM_LIBS} )

	SET_SOURCE_FILES_PROPERTIES( ${RES_GLES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
	SET_TARGET_PROPERTIES( ${APP_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PLIST_FILE} )
	
ELSE()
	ADD_EXECUTABLE( ${APP_NAME} ${CONSOLE_SYSTEM} ${SRC} ${RES} ${EXAMPLE_COMMON} )
	TARGET_LINK_LIBRARIES( ${APP_NAME} crender ${PLATFORM_LIBS} )
	
	IF( WIN32 )
		SET( RES_D3D9 ${RES} )
		STRING( REPLACE glsl hlsl RES_D3D9 "${RES_D3D9}" )
		ADD_EXECUTABLE( ${APP_NAME}-d3d9 ${CONSOLE_SYSTEM}  ${SRC} ${RES_D3D9} ${EXAMPLE_COMMON_D3D9} )
		TARGET_LINK_LIBRARIES( ${APP_NAME}-d3d9 crender-d3d9 ${PLATFORM_LIBS_D3D9} )
	ENDIF()
	
ENDIF()

ENDMACRO()

#-------------------------------------------------------------------------------
# example_cloth_simulation
SET( SRC 
	ClothSimulation.example.c
	Cloth.c
	Cloth.h
	Sphere.c
	Sphere.h
	)
	
SET( RES
	../media/Common.glsl
	../media/ClothSimulation.glsl
	../media/ShadowMap.glsl
	)

IF( APPLE )
	LIST( APPEND RES Example.xib )
ENDIF()

CREATE_EXAMPLE( example_cloth_simulation "${SRC}" "${RES}" )

#-------------------------------------------------------------------------------
# example_embedded_httpd
SET( SRC EmbeddedHttpd.example.c )

SET( RES )

IF( APPLE )
	LIST( APPEND RES Example.xib )
ENDIF()

CREATE_EXAMPLE( example_embedded_httpd "${SRC}" "${RES}" )

#-------------------------------------------------------------------------------
# example_mesh
SET( SRC Mesh.example.c )

SET( RES
	../media/Mesh.glsl
	)

IF( APPLE )
	LIST( APPEND RES Example.xib ../media/monkey.obj )
ENDIF()

CREATE_EXAMPLE( example_mesh "${SRC}" "${RES}" )

#-------------------------------------------------------------------------------
# example_mesh_performance
SET( SRC MeshPerformance.example.c )

SET( RES
	../media/MeshPerformance.glsl
	)

IF( APPLE )
	LIST( APPEND RES Example.xib ../media/monkey.obj )
ENDIF()

CREATE_EXAMPLE( example_mesh_performance "${SRC}" "${RES}" )

IF( NOT APPLE )

#-------------------------------------------------------------------------------
# example_label
SET( SRC 
	Label.example.c
	Label.c
	Label.h
	)
	
SET( RES ../media/Common.glsl )

IF( APPLE )
	LIST( APPEND RES Example.xib )
ENDIF()

CREATE_EXAMPLE( example_label "${SRC}" "${RES}" )

#-------------------------------------------------------------------------------
# example_triangle_tessellation
SET( SRC TriangleTessellation.example.c )

SET( RES
	#../media/TriangleTessellation.glsl
	)

IF( APPLE )
	LIST( APPEND RES Example.xib )
ENDIF()

CREATE_EXAMPLE( example_triangle_tessellation "${SRC}" "${RES}" )

ENDIF()