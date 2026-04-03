# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/image_processor_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/image_processor_autogen.dir/ParseCache.txt"
  "CMakeFiles/image_processor_gui_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/image_processor_gui_autogen.dir/ParseCache.txt"
  "CMakeFiles/unit_tests_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/unit_tests_autogen.dir/ParseCache.txt"
  "image_processor_autogen"
  "image_processor_gui_autogen"
  "unit_tests_autogen"
  )
endif()
