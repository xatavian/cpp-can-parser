cmake_minimum_required(VERSION 3.7.0)

project(CPP-CAN-Parser)


include(CTest)

set(CPPPARSER_INCLUDE_DIRECTORY 
	${CMAKE_CURRENT_LIST_DIR}/include
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
	
add_library(cpp-can-parser 
	${CPP_CAN_PARSER_COMPILATION_TYPE} 
	${CPPPARSER_SRC_FILES})
target_include_directories(cpp-can-parser 
	PUBLIC ${CPPPARSER_INCLUDE_DIRECTORY})

add_executable(can-parse 
	utils/can-parse/can-parse.cpp
	utils/can-parse/print-frame.cpp
	utils/can-parse/print-single-frame.cpp
	utils/can-parse/check-frame.cpp)
target_link_libraries(can-parse cpp-can-parser)