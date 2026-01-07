add_library(my-bedrock-module SHARED module.cpp)
target_link_libraries(my-bedrock-module bedrock::module-api)
