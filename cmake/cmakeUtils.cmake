


#setup external headers
MACRO(ENABLE_SYSTEM_HEADERS)
if(WIN32)
	set(EXTERNAL_HEADERS "/experimental:external /external:W0" PARENT_SCOPE)
endif(WIN32)
ENDMACRO(ENABLE_SYSTEM_HEADERS)

MACRO(ADD_EXTERNAL_HEADER ${HEADER})
if(WIN32)
set(EXTERNAL_HEADERS ${EXTERNAL_HEADERS} "/external:I ${HEADER}" PARENT_SCOPE)
endif(WIN32)
ENDMACRO(ADD_EXTERNAL_HEADER)

MACRO(FINALIZE_EXTERNAL_HEADER)
if(WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTERNAL_HEADERS}")
endif(WIN32)
ENDMACRO(FINALIZE_EXTERNAL_HEADER)

MACRO(SET_AS_HEADERS HEADERS)
set_source_files_properties(
	${HEADERS}
	PROPERTIES HEADER_FILE_ONLY TRUE
 )
ENDMACRO(SET_AS_HEADERS)

#grouping objects by name in the IDE
MACRO(SOURCE_GROUP_BY_FOLDER target)
  SET(SOURCE_GROUP_DELIMITER "/")
  SET(last_dir "")
  SET(files "")
  FOREACH(file ${SOURCE_FILES})
	file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}/{target}" ${file})
	GET_FILENAME_COMPONENT(dir "${relative_file}" PATH)
	IF (NOT "${dir}" STREQUAL "${last_dir}")
	  IF (files)
		SOURCE_GROUP("${last_dir}" FILES ${files})
	  ENDIF (files)
	  SET(files "")
	ENDIF (NOT "${dir}" STREQUAL "${last_dir}")
	SET(files ${files} ${file})
	SET(last_dir "${dir}")
  ENDFOREACH(file)
  IF (files)
	SOURCE_GROUP("${last_dir}" FILES ${files})
  ENDIF (files)
ENDMACRO(SOURCE_GROUP_BY_FOLDER)

#working directory
MACRO(SET_WORKING_DIRECTORY GIVEN_PATH)
	if(WIN32)
		set_target_properties(
		${PROJECT_NAME} PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY ${GIVEN_PATH})
	endif(WIN32)
ENDMACRO(SET_WORKING_DIRECTORY)
