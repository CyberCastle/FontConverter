#!/bin/bash

#####################################
####  Build Font Converter ONLY  ####
#####################################

printf "\n\e[36;1mBuild Font Converter...\e[0m\n"
# Clear build and dist folder, to avoid problems
if [[ -d build ]]; then
  rm -Rf build/
fi

# Clear build and dist folder, to avoid problems
if [[ -d dist ]]; then
  rm -Rf dist/
fi

#Build FontConverter
cmake -E make_directory build
cmake -E chdir build emcmake cmake ..
emmake make -C build

printf "\n\e[36;1mRunning Tests...\e[0m\n"
#Test FontConverter
cd test
node test.js
