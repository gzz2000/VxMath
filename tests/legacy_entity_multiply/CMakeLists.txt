add_executable(LegacyEntityMatrixMultiplyReference main.cpp)
if(TARGET VxMath)
    target_link_libraries(LegacyEntityMatrixMultiplyReference PRIVATE VxMath)
else()
    target_link_libraries(LegacyEntityMatrixMultiplyReference PRIVATE VxMathStatic)
endif()
add_test(NAME LegacyEntityMatrixMultiplyReference
    COMMAND LegacyEntityMatrixMultiplyReference
        "${CMAKE_CURRENT_SOURCE_DIR}/inputs.bin"
        "${CMAKE_CURRENT_SOURCE_DIR}/expected.bin")
