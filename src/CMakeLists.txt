cmake_minimum_required (VERSION 2.8.8)
project (LightBlog)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lmysqlclient -ldl -std=c++11")

include_directories(.)

find_package(Threads REQUIRED)

set(BOOST_COMPONENTS system thread filesystem date_time)
# Late 2017 TODO: remove the following checks and always use std::regex
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        set(BOOST_COMPONENTS ${BOOST_COMPONENTS} regex)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_BOOST_REGEX")
    endif()
endif()
find_package(Boost 1.53.0 COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

if(APPLE)
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()

add_executable(lightblog main.cpp blog.cpp sqlite3.c scrypt/b64.cpp scrypt/crypto_scrypt-check.cpp scrypt/crypto_scrypt-hexconvert.cpp scrypt/crypto_scrypt-nosse.cpp scrypt/crypto-mcf.cpp scrypt/sha256.cpp scrypt/slowequals.cpp bbcode/bbcode_lexer.cpp bbcode/bbcode_parser.cpp bbcode/bbcode_utils.cpp)
target_link_libraries(lightblog ${Boost_LIBRARIES})
target_link_libraries(lightblog ${CMAKE_THREAD_LIBS_INIT})

#TODO: add requirement for version 1.0.1g (can it be done in one line?)
find_package(OpenSSL)


if(MSYS) #TODO: Is MSYS true when MSVC is true?
    target_link_libraries(lightblog ws2_32 wsock32)
    if(OPENSSL_FOUND)
        target_link_libraries(https_examples ws2_32 wsock32)
    endif()
endif()

enable_testing()

install(FILES blog.hpp server_http.hpp client_http.hpp scrypt/b64.h scrypt/crypto_scrypt-hexconvert.h scrypt/libscrypt.h scrypt/sha256.h scrypt/slowequals.h scrypt/sysendian.h DESTINATION include/simple-web-server)
