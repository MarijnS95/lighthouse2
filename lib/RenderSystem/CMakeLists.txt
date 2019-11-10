project(RenderSystem)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_library(${PROJECT_NAME} ${sources} ${headers})
target_compile_definitions(${PROJECT_NAME} PRIVATE RENDERSYSTEMBUILD)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<BOOL:${MSVC}>:/arch:AVX2>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<BOOL:NOT ${MSVC}>:-mavx2 -mfma>)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
if(NOT WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
endif()
if(MSVC)
	# Prevent CUDA projects from inheriting this flag.
	target_compile_options(${PROJECT_NAME} PRIVATE /MP)
endif()
# External dependencies, PUBLIC because exposed through header:
target_link_libraries(${PROJECT_NAME} PUBLIC
	tinyxml2
	TinyGLTF::tinygltf
	TinyOBJLoader::tinyobjloader)

target_link_libraries(${PROJECT_NAME} PUBLIC Platform)

if(BUILD_SHARED_LIBS)
	install(TARGETS ${PROJECT_NAME})
endif()

include(CheckCXXSourceCompiles)

check_cxx_source_compiles ("
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
int main() {
   int fd = open(\"foo\", O_RDONLY);
   struct stat s;
   fstat(fd, &s);
   size_t len = s.st_size;
   void *ptr = mmap(0, len, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
   munmap(ptr, len);
}
" HAVE_MMAP)
if(HAVE_MMAP)
	target_compile_definitions(${PROJECT_NAME} PUBLIC PBRT_HAVE_MMAP)
endif()

iF(MSVC OR WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC PBRT_IS_WINDOWS)
endif()
