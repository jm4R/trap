include(FetchContent)

#set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL ON)

FetchContent_Declare(Catch2
    URL https://github.com/catchorg/Catch2/archive/v2.11.0.zip
    URL_HASH SHA1=e457eb862b3f9b307c0355db3d197d38bea7c785
)

FetchContent_GetProperties(Catch2)
if(NOT catch2_POPULATED)
    FetchContent_Populate(Catch2)
    add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
endif()
