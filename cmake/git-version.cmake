find_package(Git)

if(GIT_EXECUTABLE)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --tags --dirty
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
    RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  # If no error took place, save the version
  if(NOT GIT_DESCRIBE_ERROR_CODE)
    set(GIT_DESCRIBE "${GIT_DESCRIBE_VERSION}")
  endif()
endif()

if(NOT DEFINED GIT_DESCRIBE)
  set(GIT_DESCRIBE 0.0.0-0-unknown)
  message(WARNING "Failed to determine GIT_DESCRIBE from Git tags. Using default version \"${GIT_DESCRIBE}\".")
endif()
