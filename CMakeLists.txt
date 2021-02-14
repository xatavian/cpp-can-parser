cmake_minimum_required(VERSION 3.7.0)

project(CPP-CAN-Parser)

include(CTest)
include(GenerateExportHeader)


set(CPPPARSER_INCLUDE_DIRECTORY 
	${CMAKE_CURRENT_LIST_DIR}/include
)

set(CPPPARSER_INCLUDE_DIRECTORY_PRIVATE 
	${CMAKE_CURRENT_LIST_DIR}/src/parsing
)

set(CPPPARSER_SRC_FILES
	src/models/CANDatabase.cpp
	src/models/CANFrame.cpp
  	src/models/CANSignal.cpp
	src/parsing/DBCParser.cpp
	src/parsing/ParsingUtils.cpp
	src/parsing/Tokenizer.cpp
	src/analysis/CANFrameAnalysis.cpp)

set(CPP_CAN_PARSER_COMPILATION_TYPE SHARED)
if(CPP_CAN_PARSER_USE_STATIC)
	set(CPP_CAN_PARSER_COMPILATION_TYPE STATIC)
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS True)
add_library(cpp-can-parser
	${CPP_CAN_PARSER_COMPILATION_TYPE} 
	${CPPPARSER_SRC_FILES})
target_include_directories(cpp-can-parser 
	PUBLIC ${CPPPARSER_INCLUDE_DIRECTORY}
		   ${CMAKE_CURRENT_BINARY_DIR}/exports/
	PRIVATE ${CPPPARSER_INCLUDE_DIRECTORY_PRIVATE})
generate_export_header(cpp-can-parser
	BASE_NAME cpp_can_parser
	EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/exports/cpp_can_parser_export.h)
install(FILES cpp-can-parser
		${CMAKE_CURRENT_BINARY_DIR}/cpp_can_parser_export.h
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)

add_executable(can-parse 
	utils/can-parse/can-parse.cpp
	utils/can-parse/print-frame.cpp
	utils/can-parse/print-single-frame.cpp
	utils/can-parse/check-frame.cpp)
target_link_libraries(can-parse cpp-can-parser)

if(BUILD_TESTING)
	file(COPY tests/dbc-files/
		DESTINATION dbc-files/)
	
	add_executable(cpc-test-parsing
		tests/test-parsing.cpp)
	target_link_libraries(cpc-test-parsing PUBLIC cpp-can-parser)

	add_test(NAME cpc-test-parsing
			COMMAND cpc-test-parsing)

	add_test(NAME cpc-checkframe-1
			 COMMAND can-parse checkframe dbc-files/single-frame-1.dbc)

	add_test(NAME cpc-checkframe-2
			 COMMAND can-parse checkframe dbc-files/single-frame-2.dbc)

	add_test(NAME cpc-checkframe-big-endian-1
			 COMMAND can-parse checkframe 294 dbc-files/big-endian-1.dbc)

	add_test(NAME cpc-checkframe-big-endian-2
			 COMMAND can-parse checkframe 1807 dbc-files/big-endian-1.dbc)
	
	add_test(NAME cpc-checkframe-big-endian-3
			 COMMAND can-parse checkframe 1800 dbc-files/big-endian-1.dbc)
	
	add_test(NAME cpc-checkframe-decimal-scale-1
			 COMMAND can-parse checkframe dbc-files/decimal-scale-1.dbc)
endif()