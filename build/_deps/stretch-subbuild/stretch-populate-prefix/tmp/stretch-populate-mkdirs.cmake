# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/stretch-src")
  file(MAKE_DIRECTORY "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/stretch-src")
endif()
file(MAKE_DIRECTORY
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/stretch-build"
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/stretch-subbuild/stretch-populate-prefix"
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/stretch-subbuild/stretch-populate-prefix/tmp"
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/stretch-subbuild/stretch-populate-prefix/src/stretch-populate-stamp"
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/stretch-subbuild/stretch-populate-prefix/src"
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/stretch-subbuild/stretch-populate-prefix/src/stretch-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/stretch-subbuild/stretch-populate-prefix/src/stretch-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/stretch-subbuild/stretch-populate-prefix/src/stretch-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
