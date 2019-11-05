#!/bin/bash

#####################################
####  Build Font Converter ONLY  ####
#####################################

printf "\n\e[36;1mBuild Font Converter...\e[0m\n"
# Clear build and out folder, to avoid problems
if [[ -d build ]]; then
  rm -Rf build/
fi

# Clear build and out folder, to avoid problems
if [[ -d out ]]; then
  rm -Rf out/
fi

#Build FontConverter
cmake -E make_directory build
cmake -E chdir build emconfigure cmake ..
emmake make -C build

#Test FontConverter
cd test
node test.js
