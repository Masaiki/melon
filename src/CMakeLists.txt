cmake_minimum_required(VERSION 3.15)
project(melon LANGUAGES C)
set(CMAKE_C_STANDARD 99)

set(SOURCES
    melon.c iconv.c jbig.c jpeg.c zlib.c
    cnki_caj.c cnki_hn.c cnki_kdh.c cnki_outline_tree.c
    cnki_pdf.c cnki_xml.c cnki_zlib.c cnki_jbig.c cnki.c
    pdf_cnki.c pdf_get.c pdf_parser.c pdf_writer.c pdf.c
)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)
find_package(Iconv REQUIRED)

find_path(JBIGKIT_INCLUDE_DIR jbig.h)
find_library(JBIGKIT_LIBRARY NAMES libjbig)

add_executable(melon ${SOURCES})

target_include_directories(melon PRIVATE ${JBIGKIT_INCLUDE_DIR})
target_link_libraries(melon PRIVATE ZLIB::ZLIB OpenSSL::Crypto JPEG::JPEG Iconv::Iconv ${JBIGKIT_LIBRARY})