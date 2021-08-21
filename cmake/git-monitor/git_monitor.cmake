# This is a modified version of:
# https://github.com/andrew-hardin/cmake-git-version-tracking/blob/master/git_watcher.cmake

# Short hand for converting paths to absolute.
macro(PATH_TO_ABSOLUTE var_name)
    get_filename_component(${var_name} "${${var_name}}" ABSOLUTE)
endmacro()

# Check that a required variable is set.
macro(CHECK_REQUIRED_VARIABLE var_name)
    if(NOT DEFINED ${var_name})
        message(FATAL_ERROR "The \"${var_name}\" variable must be defined.")
    endif()
    PATH_TO_ABSOLUTE(${var_name})
endmacro()

# Check that an optional variable is set, or, set it to a default value.
macro(CHECK_OPTIONAL_VARIABLE var_name default_value)
    if(NOT DEFINED ${var_name})
        set(${var_name} ${default_value})
    endif()
    PATH_TO_ABSOLUTE(${var_name})
endmacro()

# Macro: RunGitCommand
# Description: short-hand macro for calling a git function. Outputs are the
#              "exit_code" and "output" variables.
macro(RunGitCommand)
    execute_process(COMMAND
        "${GIT_EXECUTABLE}" ${ARGV}
        WORKING_DIRECTORY "${_working_dir}"
        RESULT_VARIABLE exit_code
        OUTPUT_VARIABLE output
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT exit_code EQUAL 0)
        set(ENV{GIT_RETRIEVED_STATE} "false")
    endif()
endmacro()

# Function: GetGitState
# Description: gets the current state of the git repo.
# Args:
#   _working_dir (in)  string; the directory from which git commands will be executed.
function(GetGitState _working_dir)

    # This is an error code that'll be set to FALSE if the
    # RunGitCommand ever returns a non-zero exit code.
    set(ENV{GIT_RETRIEVED_STATE} "true")

    # Get whether or not the working tree is dirty.
    RunGitCommand(status --porcelain)
    if(NOT exit_code EQUAL 0)
        set(ENV{GIT_IS_DIRTY} "false")
    else()
        if(NOT "${output}" STREQUAL "")
            set(ENV{GIT_IS_DIRTY} "true")
        else()
            set(ENV{GIT_IS_DIRTY} "false")
        endif()
    endif()

    # There's a long list of attributes grabbed from git show.
    RunGitCommand(show -s "--format=%H" HEAD)
    if(exit_code EQUAL 0)
        set(ENV{GIT_HEAD_SHA1} ${output})
    endif()

    RunGitCommand(show -s "--format=%an" HEAD)
    if(exit_code EQUAL 0)
        set(ENV{GIT_AUTHOR_NAME} "${output}")
    endif()

    RunGitCommand(show -s "--format=%ae" HEAD)
    if(exit_code EQUAL 0)
        set(ENV{GIT_AUTHOR_EMAIL} "${output}")
    endif()

    RunGitCommand(show -s "--format=%ci" HEAD)
    if(exit_code EQUAL 0)
        set(ENV{GIT_COMMIT_DATE_ISO8601} "${output}")
    endif()

    RunGitCommand(show -s "--format=%s" HEAD)
    if(exit_code EQUAL 0)
        # Escape quotes
        string(REPLACE "\"" "\\\"" output "${output}")
        set(ENV{GIT_COMMIT_SUBJECT} "${output}")
    endif()

    RunGitCommand(show -s "--format=%b" HEAD)
    if(exit_code EQUAL 0)
        if(output)
            # Escape quotes
            string(REPLACE "\"" "\\\"" output "${output}")
            # Escape line breaks in the commit message.
            string(REPLACE "\r\n" "\\r\\n\\\r\n" safe "${output}")
            if(safe STREQUAL output)
                # Didn't have windows lines - try unix lines.
                string(REPLACE "\n" "\\n\\\n" safe "${output}")
            endif()
        else()
            # There was no commit body - set the safe string to empty.
            set(safe "")
        endif()
        set(ENV{GIT_COMMIT_BODY} "\"${safe}\"")
    else()
        set(ENV{GIT_COMMIT_BODY} "\"\"") # empty string.
    endif()

    # Get output of git describe
    RunGitCommand(describe --always --tags --dirty)
    if(NOT exit_code EQUAL 0)
        set(ENV{GIT_DESCRIBE} "unknown")
    else()
        if (${output} MATCHES "v*.*.*")
            string(
                REGEX REPLACE
                # Remove v from (v*.*.*)
                "v(.*\\..*\\..*)" "\\1"
                temp
                ${output}
            )
            set(ENV{GIT_DESCRIBE} "${temp}")
        else()
            set(ENV{GIT_DESCRIBE} "${output}")
        endif()
    endif()

    # >>>
    # 2. Additional git properties can be added here via the
    #    "execute_process()" command. Be sure to set them in
    #    the environment using the same variable name you added
    #    to the "_state_variable_names" list.

endfunction()

# Function: GitStateChangedAction
# Description: this function is executed when the state of the git
#              repository changes (e.g. a commit is made).
function(GitStateChangedAction)
    foreach(var_name ${_state_variable_names})
        set(${var_name} $ENV{${var_name}})
    endforeach()
    configure_file("${PRE_CONFIGURE_FILE}" "${POST_CONFIGURE_FILE}" @ONLY)
endfunction()

# Function: HashGitState
# Description: loop through the git state variables and compute a unique hash.
# Args:
#   _state (out)  string; a hash computed from the current git state.
function(HashGitState _state)
    set(ans "")
    foreach(var_name ${_state_variable_names})
        string(SHA256 ans "${ans}$ENV{${var_name}}")
    endforeach()
    set(${_state} ${ans} PARENT_SCOPE)
endfunction()

# Function: CheckGit
# Description: check if the git repo has changed. If so, update the state file.
# Args:
#   _working_dir    (in)  string; the directory from which git commands will be ran.
#   _state_changed (out)    bool; whether or no the state of the repo has changed.
function(CheckGit _working_dir _state_changed)

    # Get the current state of the repo.
    GetGitState("${_working_dir}")

    # Convert that state into a hash that we can compare against
    # the hash stored on-disk.
    HashGitState(state)

    # Issue 14: post-configure file isn't being regenerated.
    #
    # Update the state to include the SHA256 for the pre-configure file.
    # This forces the post-configure file to be regenerated if the
    # pre-configure file has changed.
    file(SHA256 ${PRE_CONFIGURE_FILE} preconfig_hash)
    string(SHA256 state "${preconfig_hash}${state}")

    # Check if the state has changed compared to the backup on disk.
    if(EXISTS "${GIT_STATE_FILE}")
        file(READ "${GIT_STATE_FILE}" OLD_HEAD_CONTENTS)
        if(OLD_HEAD_CONTENTS STREQUAL "${state}")
            # State didn't change.
            set(${_state_changed} "false" PARENT_SCOPE)
            return()
        endif()
    endif()

    # The state has changed.
    # We need to update the state file on disk.
    # Future builds will compare their state to this file.
    file(WRITE "${GIT_STATE_FILE}" "${state}")
    set(${_state_changed} "true" PARENT_SCOPE)
endfunction()

CHECK_REQUIRED_VARIABLE(PRE_CONFIGURE_FILE)
CHECK_REQUIRED_VARIABLE(POST_CONFIGURE_FILE)
CHECK_OPTIONAL_VARIABLE(GIT_WORKING_DIR "${CMAKE_SOURCE_DIR}")

if(NOT DEFINED GIT_EXECUTABLE)
    find_package(Git QUIET REQUIRED)
endif()
CHECK_REQUIRED_VARIABLE(GIT_EXECUTABLE)

set(_state_variable_names
    GIT_RETRIEVED_STATE
    GIT_HEAD_SHA1
    GIT_IS_DIRTY
    GIT_AUTHOR_NAME
    GIT_AUTHOR_EMAIL
    GIT_COMMIT_DATE_ISO8601
    GIT_COMMIT_SUBJECT
    GIT_COMMIT_BODY
    GIT_DESCRIBE
    # >>>
    # 1. Add the name of the additional git variable you're interested in monitoring
    #    to this list.
)

CheckGit("${GIT_WORKING_DIR}" changed)
if(changed OR NOT EXISTS "${POST_CONFIGURE_FILE}")
    GitStateChangedAction()
endif()
