# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/signalsmith-linear-src")
  file(MAKE_DIRECTORY "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/signalsmith-linear-src")
endif()
file(MAKE_DIRECTORY
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/signalsmith-linear-build"
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/signalsmith-linear-subbuild/signalsmith-linear-populate-prefix"
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/signalsmith-linear-subbuild/signalsmith-linear-populate-prefix/tmp"
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/signalsmith-linear-subbuild/signalsmith-linear-populate-prefix/src/signalsmith-linear-populate-stamp"
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/signalsmith-linear-subbuild/signalsmith-linear-populate-prefix/src"
  "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/signalsmith-linear-subbuild/signalsmith-linear-populate-prefix/src/signalsmith-linear-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/signalsmith-linear-subbuild/signalsmith-linear-populate-prefix/src/signalsmith-linear-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/kyle.ramsey/CLionProjects/SpectralShifter/build/_deps/signalsmith-linear-subbuild/signalsmith-linear-populate-prefix/src/signalsmith-linear-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
