# SPDX-FileCopyrightText: 2024 Ralf Habacker <ralf.habacker@freenet.de>
# SPDX-License-Identifier: MIT

find_program(DIFF_EXECUTABLE diff)

execute_process(
  COMMAND ${launcher} --${format} --${type} --report ${report} --output ${outfilename} ${testfile}
  COMMAND_ECHO STDERR
  RESULT_VARIABLE run_result
  WORKING_DIRECTORY ${workingdir}
)

if(NOT EXISTS ${reffilename})
  message(ERROR "could not compare files - reference file '${reffilename}' is missing")
  if(WIN32)
      set(msg_prefix "set LANG=C\n")
  else()
      set(msg_prefix "LANG=C ")
  endif()
  message(STATUS "run '${msg_prefix}${launcher} --reference --${format} --${type} --report \"${report}\" --output \"${reffilename}\" ${testfile}' to generate it")
  return()
endif()
execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol ${reffilename} ${outfilename}
  COMMAND_ECHO STDERR
  RESULT_VARIABLE compare_result
)
if(compare_result EQUAL 0)
  message(STATUS "The file is identical.")
elseif(compare_result EQUAL 1)
  if(DIFF_EXECUTABLE)
    execute_process(COMMAND ${DIFF_EXECUTABLE} ${reffilename} ${outfilename})
  else()
    # use build in diff as fallback
    file(STRINGS ${reffilename} reffile)
    file(STRINGS ${outfilename} outfile)
    list(LENGTH reffile reffile_length)
    list(LENGTH outfile outfile_length)
    if (NOT reffile_length EQUAL outfile_length)
      message(FATAL_ERROR " The number of lines is different.")
    endif()
    if (reffile_length EQUAL 0)
      message(FATAL_ERROR " No lines in reference file found.")
    endif()
    set(i 0)
    set(difflines 0)
    while(i LESS reffile_length)
      list(GET reffile ${i} refline)
      list(GET outfile ${i} outline)
      if (NOT refline STREQUAL outline)
        math(EXPR difflines "${difflines} + 1")
        message("-${i}: ${refline}")
        message("+${i}: ${outline}")
      endif()
      math(EXPR i "${i} + 1")
    endwhile()
    if(WIN32 AND difflines EQUAL 0)
      message(STATUS " Ignore incorrect comparison result of cmake, although no lines are different.")
      return()
    endif()
  endif()
  message(FATAL_ERROR " The file is different.")
else()
  message(FATAL_ERROR " Error while comparing the file.")
endif()
