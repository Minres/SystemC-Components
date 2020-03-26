if(__git_functions)
    return()
endif()
set(__git_functions YES)

function( get_info_from_git )
	# get the link of remote
#    execute_process(
#        COMMAND git remote -v
#        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#        RESULT_VARIABLE   git_result
#        OUTPUT_VARIABLE   git_branch
#        ERROR_VARIABLE    git_error
#        OUTPUT_STRIP_TRAILING_WHITESPACE
#        ERROR_STRIP_TRAILING_WHITESPACE
#    )
#    if( NOT git_result EQUAL 0 )
#        message( FATAL_ERROR "Failed to execute Git: ${git_error}")
#    else()
#        set(GIT_URL ${git_branch} PARENT_SCOPE)
#    endif()
	# get the branch name
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE   git_result
        OUTPUT_VARIABLE   git_branch
        ERROR_VARIABLE    git_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if( NOT git_result EQUAL 0 )
        message( FATAL_ERROR "Failed to execute Git: ${git_error}")
    else()
        set(GIT_BRANCH ${git_branch} PARENT_SCOPE)
    endif()
    # get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE   git_result
        OUTPUT_VARIABLE   git_commit_hash
        ERROR_VARIABLE    git_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if( NOT git_result EQUAL 0 )
        message( FATAL_ERROR "Failed to execute Git: ${git_error}")
    else()
        set(GIT_COMMIT_SHA ${git_commit_hash} PARENT_SCOPE)
    endif()
    # get the latest commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE   git_result
        OUTPUT_VARIABLE   git_commit_hash
        ERROR_VARIABLE    git_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if( NOT git_result EQUAL 0 )
        message( FATAL_ERROR "Failed to execute Git: ${git_error}")
    else()
        set(GIT_COMMIT_HASH ${git_commit_hash} PARENT_SCOPE)
    endif()
    # get the last tag 
    execute_process(
        COMMAND           git describe --tags --abbrev=0
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE   git_result
        OUTPUT_VARIABLE   git_tag
        ERROR_VARIABLE    git_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if( (NOT git_result EQUAL 0 ) AND (NOT git_result EQUAL 128))
        message( FATAL_ERROR "Failed to execute Git: ${git_error}")
    else()
        set(GIT_TAG ${git_tag} PARENT_SCOPE)
    endif()
endfunction()
    