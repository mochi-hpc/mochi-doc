add_library(bedrock-module-b SHARED module.cpp)
target_link_libraries(bedrock-module-b bedrock::module-api)
