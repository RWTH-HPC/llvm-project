#
# ompt-critical-path.cpp -- Critical
#
#
# //===----------------------------------------------------------------------===//
# //
# // Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# // See tools/archer/LICENSE.txt for details.
# // SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# //
# //===----------------------------------------------------------------------===//
  
  

include_directories(${LIBOMP_INCLUDE_DIR})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(MPI COMPONENTS C)

find_package(Threads)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-Xlinker -v -v")

include_directories(../multiplex/)

add_library(ompt_criticalpath SHARED critical-core.cpp ompt-critical.cpp)

message(STATUS "MPI_FOUND = ${MPI_FOUND}")
if(MPI_FOUND)
  include_directories(${MPI_C_INCLUDE_DIRS})
  add_library(ompt_mpicriticalpath SHARED critical-core.cpp ompt-critical.cpp mpi-critical.cpp)
  target_link_libraries(ompt_mpicriticalpath ${MPI_LIBRARIES} ${CMAKE_DL_LIBS})
  target_link_libraries(ompt_mpicriticalpath ${CMAKE_THREAD_LIBS_INIT})
  target_compile_definitions(ompt_mpicriticalpath PUBLIC USE_MPI MPICH_SKIP_MPICXX)
#  target_compile_options(ompt_mpicriticalpath PUBLIC -fsanitize=address)
#  target_link_options(ompt_mpicriticalpath PUBLIC -fsanitize=address)

  add_library(ompt_fakempicriticalpath SHARED critical-core.cpp ompt-critical.cpp mpi-critical.cpp)
  target_link_libraries(ompt_fakempicriticalpath ${MPI_LIBRARIES} ${CMAKE_DL_LIBS})
  target_link_libraries(ompt_fakempicriticalpath ${CMAKE_THREAD_LIBS_INIT})
  target_compile_definitions(ompt_fakempicriticalpath PUBLIC USE_MPI MPICH_SKIP_MPICXX FAKEMPI)

  install(TARGETS ompt_mpicriticalpath ompt_fakempicriticalpath 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
endif()

target_link_libraries(ompt_criticalpath ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ompt_criticalpath 
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

#add_subdirectory(tests)
