set(infotable_test_files
  test-general-information.xml
)

set(querytable_test_files
  test-382245.xml
  test-423870.xml
  test-424674.xml
  test-448788.xml
  test-453033.xml
  test-469195.xml
  test-cash-flow.xml
  test-investment.xml
  test-net-worth.xml
  test-transactions.xml
  refinance-loan.xml
)

set(pivottable_test_files
  test-income-and-expenses.xml
  test-net-worth.xml
  test-pivottable.xml
)

macro(fetch_report_names file)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs RETURN_NAMES RETURN_TYPES RETURN_IDS)
  cmake_parse_arguments(FRN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(${FRN_RETURN_NAMES})
  set(${FRN_RETURN_TYPES})
  set(${FRN_RETURN_IDS})
  file(STRINGS
    ${file} lines
    REGEX ".*<REPORT .*"
    ENCODING UTF-8
  )
  foreach(line ${lines})
    string(REGEX REPLACE "<REPORT (.*)[/]?>" "\\1" l ${line})
    #message( "++++ ${l}")
    string(REGEX REPLACE " ([a-zA-Z0-9]+=)" ";\\1" attributes ${l})
    foreach(m ${attributes})
      if (m MATCHES "^name=\"(.*)\"")
        list(APPEND ${FRN_RETURN_NAMES} ${CMAKE_MATCH_1})
      endif()
      if (m MATCHES "^type=\"(.*)\"")
        list(APPEND ${FRN_RETURN_TYPES} ${CMAKE_MATCH_1})
      endif()
      if (m MATCHES "^id=\"(.*)\"")
          list(APPEND ${FRN_RETURN_IDS} ${CMAKE_MATCH_1})
      endif()
    endforeach()
  endforeach()
endmacro()

macro(to_test_and_file_name)
  set(options)
  set(oneValueArgs TYPE BASENAME REPORT_NAME)
  set(multiValueArgs RETURN_TEST_NAME RETURN_FILE_NAME)
  cmake_parse_arguments(TTAFN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  string(REGEX REPLACE "[ .]" "-" _rn ${TTAFN_REPORT_NAME})
  string(REGEX REPLACE "[^. a-zA-Z0-9_-]" "_" _rn ${_rn})
  set(${TTAFN_RETURN_TEST_NAME} reports-${TTAFN_TYPE}-${TTAFN_BASENAME}-${_rn})
  set(${TTAFN_RETURN_FILE_NAME} ${TTAFN_BASENAME}-report-${TTAFN_TYPE}-${_rn})
endmacro()

macro(add_report_tests)
   set(options)
   set(oneValueArgs TYPE)
   set(multiValueArgs FILES FORMATS)
   cmake_parse_arguments(ART "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  foreach(file ${ART_FILES})
    cmake_path(GET file STEM basename)
    message(STATUS "${file}")
    fetch_report_names(${file} RETURN_NAMES report_names RETURN_TYPES report_types RETURN_IDS report_ids)
    set(i 0)
    list(LENGTH report_names j)
    while(i LESS j)
      list(GET report_names ${i} rn)
      list(GET report_types ${i} rt)
      list(GET report_ids ${i} id)
      math(EXPR i "${i} + 1")
      if(NOT rt MATCHES "${ART_TYPE}.*")
        continue()
      endif()
      foreach(format ${ART_FORMATS})
        to_test_and_file_name(TYPE ${ART_TYPE} BASENAME ${basename} REPORT_NAME ${rn} RETURN_TEST_NAME test_name RETURN_FILE_NAME file_name)
        message(" * adding test '${test_name}-${format}' for report '${rn}' with type '${rt}' and id '${id}'")
        add_test(
          NAME ${test_name}-${format}
          COMMAND ${CMAKE_COMMAND}
            -Dlauncher=$<TARGET_FILE:kmmreport-export>
            -Dtestfile=${CMAKE_CURRENT_SOURCE_DIR}/${file}
            -Dformat=${format}
            -Dtype=${ART_TYPE}
            -Dreport=${rn}
            -Doutfilename=${CMAKE_CURRENT_BINARY_DIR}/${file_name}.${format}
            -Dreffilename=${CMAKE_CURRENT_SOURCE_DIR}/${file_name}.${format}
            -Dworkingdir=${CMAKE_CURRENT_BINARY_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/compare.cmake
        )
        set_tests_properties(${test_name}-${format}
          PROPERTIES
            SKIP_REGULAR_EXPRESSION "is missing" "SKIP" "Skipped"
            ENVIRONMENT LANG=C
          )
      endforeach()
    endwhile()
  endforeach()
endmacro()

add_report_tests(TYPE infotable FILES ${infotable_test_files} FORMATS csv)
add_report_tests(TYPE querytable FILES ${querytable_test_files} FORMATS csv)
add_report_tests(TYPE pivottable FILES ${pivottable_test_files} FORMATS csv)
