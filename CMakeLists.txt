cmake_minimum_required(VERSION 3.24)
project(Giggle C)

cmake_policy(SET CMP0135 NEW)

include(ExternalProject)
## ---------------------------------------------------------------------
## HTSLIB 
## ---------------------------------------------------------------------
set(HTSLIB_VERSION "1.21")

ExternalProject_Add(htslib
    URL https://github.com/samtools/htslib/releases/download/1.21/htslib-1.21.tar.bz2
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/htslib
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/htslib
    CONFIGURE_COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/htslib && autoreconf -i && ./configure --disable-bz2 --disable-lzma --enable-libcurl
    BUILD_COMMAND make -C ${CMAKE_CURRENT_BINARY_DIR}/htslib
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    INSTALL_COMMAND ""
    PREFIX ""
)

add_library(hts STATIC IMPORTED)
set_target_properties(hts PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/htslib/libhts.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/htslib
)

add_dependencies(htslib hts)

## ---------------------------------------------------------------------
## GIGGLE
## ---------------------------------------------------------------------
# set(BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
# set(OBJ_DIR "${CMAKE_CURRENT_BINARY_DIR}/obj")
# set(LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/src")
set(HTS_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/htslib")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -D_FILE_OFFSET_BITS=64 -Werror -Wuninitialized -DBLOCK_STORE")

include_directories(${CMAKE_CURRENT_BINARY_DIR}/htslib)

set(LIBS dl z m pthread curl crypto)

set(LIB_SRC
    "${SRC_DIR}/bpt.c"
    "${SRC_DIR}/disk_file_header.c"
    "${SRC_DIR}/disk_store.c"
    "${SRC_DIR}/zlib_wrapper.c"
    "${SRC_DIR}/fastlz_wrapper.c"
    "${SRC_DIR}/cache.c"
    "${SRC_DIR}/data_reg.c"
    "${SRC_DIR}/file_read.c"
    "${SRC_DIR}/giggle_index.c"
    "${SRC_DIR}/lists.c"
    "${SRC_DIR}/ll.c"
    "${SRC_DIR}/util.c"
    "${SRC_DIR}/timer.c"
    "${SRC_DIR}/wah.c"
    "${SRC_DIR}/kfunc.c"
    "${SRC_DIR}/index.c"
    "${SRC_DIR}/offset_index.c"
    "${SRC_DIR}/metadata_index.c"
    "${SRC_DIR}/query_filter.c"
    "${SRC_DIR}/search.c"
    "${SRC_DIR}/leaf.c"
    "${SRC_DIR}/pq.c"
    "${SRC_DIR}/jsw_avltree.c"
    "${SRC_DIR}/fastlz.c"
)

add_library(libgiggle STATIC ${LIB_SRC})
target_include_directories(libgiggle PRIVATE ${HTS_INCLUDE_DIR})

set(PROGRAMS giggle api_test offset_idx_lookup)
foreach(PROG ${PROGRAMS})
    add_executable(${PROG} "${SRC_DIR}/${PROG}.c")
    target_link_libraries(${PROG} libgiggle hts ${LIBS})
    target_compile_definitions(${PROG} PRIVATE -DSAMTOOLS=1)
endforeach()

add_executable(server_enrichment ${SRC_DIR}/server_enrichment.c)
target_include_directories(server_enrichment PRIVATE ${HTS_INCLUDE_DIR})
target_link_libraries(server_enrichment libgiggle hts microhttpd ${LIBS})

