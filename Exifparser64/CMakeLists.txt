# CMakeList.txt : CMake project for Exifparser64, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.3)

project ("Exifparser64")

# Add source to this project's executable.
add_library (Exifparser64 SHARED "dllmain.cpp" "ExifParser.cpp")

# TODO: Add tests and install targets if needed.
