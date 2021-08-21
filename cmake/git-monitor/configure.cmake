set(GIT_MONITOR_DIR ${CMAKE_CURRENT_LIST_DIR})

function(SetupGitMonitor _target _pre_conf _post_conf)

    set(git_state "${CMAKE_BINARY_DIR}/git-state-hash")
    add_custom_target(git_monitor
        ALL
        DEPENDS ${_pre_conf}
        BYPRODUCTS
            ${_post_conf}
            ${git_state}
        COMMENT "Checking the git repository for changes..."
        COMMAND
            ${CMAKE_COMMAND}
            -D_BUILD_TIME_CHECK_GIT=TRUE
            -DGIT_STATE_FILE=${git_state}
            -DPRE_CONFIGURE_FILE=${_pre_conf}
            -DPOST_CONFIGURE_FILE=${_post_conf}
            -P "${GIT_MONITOR_DIR}/git_monitor.cmake"
    )

    add_dependencies(
        ${_target}
        git_monitor
    )

endfunction()

function(GetGitDescribe _out_version)

    set(${_out_version} "0.0.0" PARENT_SCOPE)

    set_property(
        DIRECTORY APPEND
        # Force cmake to rebuild if any commit or tag is changed.
        PROPERTY CMAKE_CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/.git/index"
            "${CMAKE_CURRENT_SOURCE_DIR}/.git/refs/tags"
    )

    execute_process(
        COMMAND git describe --tags --always --dirty
        RESULT_VARIABLE GIT_DESCRIBE_RESULT
        OUTPUT_VARIABLE GIT_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    message(STATUS "${CMAKE_PROJECT_NAME} git description: '${GIT_DESCRIBE}'")

    if ("${GIT_DESCRIBE}" STREQUAL "")
        message(WARNING "No git description was found! Make sure a tag precedes the current commit.")
        return()
    endif()

    set(${_out_version} ${GIT_DESCRIBE} PARENT_SCOPE)

endfunction()

