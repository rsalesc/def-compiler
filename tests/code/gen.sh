#!/bin/bash

RED='\033[1;31m'
GREEN='\033[1;32m'
CYAN='\033[1;36m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

compiler=$(cat compiler_path)

for problem_folder in *.p/; do
  problem=${problem_folder/\.p\//}
  output_fn="${problem}.p/out"
  def_fn="${problem}.p/sol.def"
  code_fn="${problem}.p/sol.c"

  echo -e "- Processing problem ${PURPLE}${problem}${NC}..."
  echo "Compiling Def code"
  ${compiler} ${def_fn} > /dev/null

  if [ $? -ne 0 ]; then
    echo -e "${RED}CE${NC}: def code does not compile"
    exit 1
  fi

  echo "Generating equivalent C code"
  ./cdef < ${def_fn} > ${code_fn}

  echo "Compiling C code"
  gcc ${code_fn}

  if [ $? -ne 0 ]; then
    echo -e "${RED}CE${NC}: C code does not compile"
    exit 1
  fi

  echo "Running C code and generating output"
  ./a.out > ${output_fn}

  if [ $? -ne 0 ]; then
    echo -e "${CYAN}RTE${NC}: C program crashes somehow, figure it out"
    exit 1
  fi

  echo;
done
