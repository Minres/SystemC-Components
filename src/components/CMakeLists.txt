project(components VERSION ${scc_VERSION} LANGUAGES CXX)

find_package(flatbuffers CONFIG QUIET)

if(CMAKE_CXX_STANDARD GREATER_EQUAL 17)
    add_library(${PROJECT_NAME} scc/at_router/nb_router.cpp scc/nu_router.cpp)
    add_library(scc::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wno-long-long -pedantic)
    endif()

    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
    )
    target_link_libraries(${PROJECT_NAME} PUBLIC scc-sysc scc-util)

    if(TARGET flatbuffers::flatbuffers)
        option(REGENERATE_FB_HEADERS "Enables the update of src/components/scc/tcp4tlm/messages_generated.h by the flatbuffer compiler flatc" OFF)
        find_package(Boost REQUIRED COMPONENTS thread chrono)
        if(REGENERATE_FB_HEADERS)
            # Generate the FlatBuffers header alongside the TCP/TLM schema.
            set(FB_SCHEMA "${CMAKE_CURRENT_SOURCE_DIR}/scc/tcp4tlm/messages.fbs")
            set(FB_OUTDIR "${CMAKE_CURRENT_SOURCE_DIR}/scc/tcp4tlm")
            build_flatbuffers("${FB_SCHEMA}" "" scc_fbschemas "" "${FB_OUTDIR}" "" "")
            add_dependencies(${PROJECT_NAME} scc_fbschemas)
        endif()

        target_sources(${PROJECT_NAME} PRIVATE scc/tcp4tlm_bridge.cpp scc/tcp4tlm_server.cpp scc/tcp4tlm_client.cpp)
        target_include_directories(${PROJECT_NAME} PRIVATE
            $<BUILD_INTERFACE:${flatbuffers_INCLUDE_DIR}> # for headers when building
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
        target_link_libraries(${PROJECT_NAME} PUBLIC flatbuffers::flatbuffers)
        if(TARGET Boost::thread)
            target_link_libraries(${PROJECT_NAME} PUBLIC Boost::thread Boost::chrono)
        else()
            target_link_libraries(${PROJECT_NAME} PUBLIC
                ${Boost_THREAD_LIBRARY}
                ${Boost_CHRONO_LIBRARY}
            )
        endif()
    endif()
else()
    add_library(${PROJECT_NAME} INTERFACE)
    add_library(scc::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
    target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
    )
    target_link_libraries(${PROJECT_NAME} INTERFACE scc-sysc)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    # VERSION ${PROJECT_VERSION}
    # FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scc_components.h
)

install(TARGETS ${PROJECT_NAME} COMPONENT components EXPORT ${PROJECT_NAME}-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${SCC_CMAKE_CONFIG_DIR}
    NAMESPACE scc::
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scc COMPONENT components
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
    FILES_MATCHING # install only matched files
    PATTERN "*.h" # select header files
    PATTERN "*.fbs" # install FlatBuffers schemas
)
