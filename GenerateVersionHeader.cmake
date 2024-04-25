if(GIT_EXECUTABLE)
	get_filename_component(SRC_DIR ${SRC} DIRECTORY)
	# Get lastest tag
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 --match "v*" --tags
		WORKING_DIRECTORY ${SRC_DIR}
		OUTPUT_VARIABLE GIT_TAG
      RESULT_VARIABLE GIT_ERROR_CODE
      OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	if(NOT GIT_ERROR_CODE)
		# Get the version number from the tag
		string(REPLACE "v" "" RELEASE ${GIT_TAG})
		# get the time of the tag
		execute_process(
			COMMAND ${GIT_EXECUTABLE} show -s --format=%ai ${GIT_TAG}
			WORKING_DIRECTORY ${SRC_DIR}
			OUTPUT_VARIABLE GIT_TAG_TIME
			RESULT_VARIABLE GIT_ERROR_CODE
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)
		if(NOT GIT_ERROR_CODE)
			set(RELEASE_TIME ${GIT_TAG_TIME})
		endif()
		# get the hash of the tag
		execute_process(
			COMMAND ${GIT_EXECUTABLE} rev-parse ${GIT_TAG}
         WORKING_DIRECTORY ${SRC_DIR}
			OUTPUT_VARIABLE GIT_TAG_HASH
         RESULT_VARIABLE GIT_ERROR_CODE
         OUTPUT_STRIP_TRAILING_WHITESPACE
         )
		if(NOT GIT_ERROR_CODE)
			# get the tag description
			execute_process(
				COMMAND ${GIT_EXECUTABLE} cat-file -p ${GIT_TAG_HASH}
				WORKING_DIRECTORY ${SRC_DIR}
				OUTPUT_VARIABLE GIT_TAG_DESCRIPTION
				RESULT_VARIABLE GIT_ERROR_CODE
				OUTPUT_STRIP_TRAILING_WHITESPACE
				)
			if(NOT GIT_ERROR_CODE)
				string(REGEX MATCHALL "[^\n\r]+" MYLINES ${GIT_TAG_DESCRIPTION})
				list(GET MYLINES -1 RELEASE_NAME)
			endif()
		endif()
	endif() # if got GIT_TAG
	# get branch name
	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
		WORKING_DIRECTORY ${SRC_DIR}
		OUTPUT_VARIABLE GIT_BRANCH
		RESULT_VARIABLE GIT_ERROR_CODE
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	if(NOT GIT_ERROR_CODE)
		set(GIT_BRANCH ${GIT_BRANCH})
	endif()
	# get commit
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --tags
		WORKING_DIRECTORY ${SRC_DIR}
		OUTPUT_VARIABLE GIT_COMMIT
		RESULT_VARIABLE GIT_ERROR_CODE
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	if(NOT GIT_ERROR_CODE)
		set(GIT_COMMIT ${GIT_COMMIT})
		# get time of commit
		execute_process(
			COMMAND ${GIT_EXECUTABLE} show -s --format=%ai ${GIT_COMMIT}
			WORKING_DIRECTORY ${SRC_DIR}
			OUTPUT_VARIABLE GIT_COMMIT_TIME
			RESULT_VARIABLE GIT_ERROR_CODE
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)
		if(NOT GIT_ERROR_CODE)
			set(GIT_COMMIT_TIME ${GIT_COMMIT_TIME})
		endif()
	endif()
endif()

# Final fallback: Just use a bogus version string that is semantically older
# than anything else and spit out a warning to the developer.
if(NOT DEFINED RELEASE)
	set(RELEASE 0.0.0)
	message(WARNING "Failed to determine RELEASE from Git tags. Using default version \"${RELEASE}\".")
endif()

configure_file(${SRC} ${DST} @ONLY)
