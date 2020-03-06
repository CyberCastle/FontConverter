#!/bin/bash

#####################################
#### Wasm generator for Freetype ####
#####################################
# Logic this script based in this wiki page: https://github.com/metafloor/bwip-js/wiki/Compiling-FreeType
# Info for appply color to this script, obtained from here: http://www.andrewnoske.com/wiki/Bash_-_adding_color

# <<<--- Begin Error Handler --->>> #
# Piece of code obtain from here: https://gist.github.com/ahendrix/7030300

# Setting errtrace allows our ERR trap handler to be propagated to functions,
# expansions and subshells
set -o errtrace

# Trap ERR to provide an error handler whenever a command exits nonzero
# this is a more verbose version of set -o errexit
trap 'stackTrace' ERR

# Stack trace function
function stackTrace() {
  local err=$?
  set +o xtrace
  local code="${1:-1}"
  echo "Error in ${BASH_SOURCE[1]}:${BASH_LINENO[0]}. '${BASH_COMMAND}' exited with status $err"
  # Print out the stack trace described by $function_stack  
  if [ ${#FUNCNAME[@]} -gt 2 ]
  then
    echo "Call tree:"
    for ((i=1;i<${#FUNCNAME[@]}-1;i++))
    do
      echo " $i: ${BASH_SOURCE[$i+1]}:${BASH_LINENO[$i]} ${FUNCNAME[$i]}(...)"
    done
  fi
  exit "${code}"
}
#  <<<--- End Error Handler --->>>  #

printf "\e[36;1mDownloading FreeType Library...\e[0m\n"

# Clone freetype2 git repository
pushd  "$(dirname "$0")/vendor" >/dev/null
if [[ -d freetype2 ]]; then
  # Delete old folder, to avoid problems
  rm -Rf freetype2/
fi

git clone git://git.sv.nongnu.org/freetype/freetype2.git
popd >/dev/null

printf "\n\e[36;1mApplying patches to FreeType Library...\e[0m\n"

# Delete exports.mk and create it empty
rm "$(dirname "$0")/vendor/freetype2/builds/exports.mk"
touch "$(dirname "$0")/vendor/freetype2/builds/exports.mk"

# Apply patches
patch "$(dirname "$0")/vendor/freetype2/include/freetype/config/ftoption.h" < "$(dirname "$0")/patches/ftoption.h.patch"

# Delete CMakeLists.txt and replace it with a customized
rm "$(dirname "$0")/vendor/freetype2/CMakeLists.txt"
cp "$(dirname "$0")/build-tools/CMakeLists.custom.txt" "$(dirname "$0")/vendor/freetype2/CMakeLists.txt"

printf "\n\e[36;1mBuild FreeType Library...\e[0m\n"
# Build FreeType, how static library
pushd  "$(dirname "$0")/vendor/freetype2" >/dev/null
cmake -E make_directory build
cmake -E chdir build emconfigure cmake ..
emmake make -C build
popd >/dev/null

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
cmake -E chdir build emconfigure cmake ..
emmake make -C build

printf "\n\e[36;1mRunning Tests...\e[0m\n"
#Test FontConverter
cd test
node test.js