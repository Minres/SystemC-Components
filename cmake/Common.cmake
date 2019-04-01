# Function to link between sub-projects
function(add_dependent_subproject subproject_name)
	#if (NOT TARGET ${subproject_name}) # target unknown
    if(NOT PROJECT_${subproject_name}) # var unknown because we build only this subproject
        find_package(${subproject_name} CONFIG REQUIRED)
    else () # we know the target thus we are doing a build from the top directory
        include_directories(../${subproject_name}/incl)
    endif ()
endfunction(add_dependent_subproject)

    # Make sure we tell the topdir CMakeLists that we exist (if build from topdir)
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(PROJECT_${PROJECT_NAME} true PARENT_SCOPE)
endif()

# Function to link between sub-projects
function(add_dependent_header subproject_name)
    include_directories(../${subproject_name}/incl)
endfunction(add_dependent_header)