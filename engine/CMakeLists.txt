cmake_minimum_required(VERSION 3.13.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include (${CMAKE_SOURCE_DIR}/cmake/cmakeUtils.cmake)

project(engine)   

	if(UNIX)
    find_package(XCB REQUIRED)
	endif(UNIX)
	find_package(Vulkan REQUIRED)

    #looking for  files
    file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.h" "src/*.inl")

	include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src ${Vulkan_INCLUDE_DIRS} 
	 "../external/headers"
	)

	#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	#	add_compile_options(/wd4251 /wd4275)
	#endif()

	#lets add specific windows build definitions
	if (WIN32)
		add_compile_definitions( _UNICODE VK_NO_PROTOTYPES VK_USE_PLATFORM_WIN32_KHR WIN32_LEAN_AND_MEAN 
		NOMINMAX QUIT_ESCAPE VULKAN_OBJ_TRACKER CP_WINDOWS_PLATFORM)
	endif (WIN32)
    if (UNIX)
        add_compile_definitions( VK_NO_PROTOTYPES QUIT_ESCAPE VULKAN_OBJ_TRACKER CP_LINUX_PLATFORM)
    endif (UNIX)

	#making sure to add the common cpp flags, that are defined in the main cpp file
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

	SOURCE_GROUP_BY_FOLDER(vulkanViewport)

	link_directories(${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>)

    #adding the executable
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${INCLUDES_FILES} 
	)

	SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

	if(WIN32)
	target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
    elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES} X11 GL ${XCB_LIBRARIES} )
    endif()
