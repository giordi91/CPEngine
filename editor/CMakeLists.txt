cmake_minimum_required(VERSION 3.13.0)


project(editor)   

	find_package(Vulkan REQUIRED)
    #looking for  files
    file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.h" "src/*.inl")

	include_directories(${CMAKE_SOURCE_DIR}/engine/src ${CMAKE_CURRENT_SOURCE_DIR}/src ${Vulkan_INCLUDE_DIRS} )

	#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	#	add_compile_options(/wd4251 /wd4275)
	#endif()

	#lets add specific windows build definitions
	if (WIN32)
		add_compile_definitions( _UNICODE VK_NO_PROTOTYPES VK_USE_PLATFORM_WIN32_KHR WIN32_LEAN_AND_MEAN 
		NOMINMAX QUIT_ESCAPE VULKAN_OBJ_TRACKER )
	endif (WIN32)

	#making sure to add the common cpp flags, that are defined in the main cpp file
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

	SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

	SOURCE_GROUP_BY_FOLDER(vulkanViewport)


	link_directories(${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>)
    #adding the executable
    add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${INCLUDES_FILES})

	SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

	target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES} engine)

	#setting working directory
	if(WIN32)
		set_target_properties(
		${PROJECT_NAME} PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>)
	endif(WIN32)

	file(COPY "../data" DESTINATION "${CMAKE_BINARY_DIR}/bin" )

