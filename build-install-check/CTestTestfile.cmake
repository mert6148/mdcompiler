# CMake generated Testfile for 
# Source directory: C:/Users/mertd/OneDrive/Masaüstü/mdcompiler
# Build directory: C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/build-install-check
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(cli_help "C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/build-install-check/Debug/mdcompiler.exe" "--help")
  set_tests_properties(cli_help PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/CMakeLists.txt;18;add_test;C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(cli_help "C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/build-install-check/Release/mdcompiler.exe" "--help")
  set_tests_properties(cli_help PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/CMakeLists.txt;18;add_test;C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(cli_help "C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/build-install-check/MinSizeRel/mdcompiler.exe" "--help")
  set_tests_properties(cli_help PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/CMakeLists.txt;18;add_test;C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(cli_help "C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/build-install-check/RelWithDebInfo/mdcompiler.exe" "--help")
  set_tests_properties(cli_help PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/CMakeLists.txt;18;add_test;C:/Users/mertd/OneDrive/Masaüstü/mdcompiler/CMakeLists.txt;0;")
else()
  add_test(cli_help NOT_AVAILABLE)
endif()
