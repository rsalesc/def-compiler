#!/bin/bash

RED='\033[1;31m'
GREEN='\033[1;32m'
CYAN='\033[1;36m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

compiler=$(cat compiler_path)
tmp_code=/tmp/cdef_code
tmp_out=/tmp/cdef_out
tmp_log=/tmp/cdef_log
simulator=spim

if [ ! -s "$compiler" ]; then
    echo -e "Compiler was not found"
    exit 1
fi

for problem_folder in *.p/; do
  problem=${problem_folder/\.p\//}
  def_fn="${problem}.p/sol.def"
  output_fn="${problem}.p/out"
  mips_fn="${problem}.p/sol.s"
  yours_fn="${problem}.p/yours.out"
  tmp_exec=$mips_fn

  echo -e "- Compiling solution for ${PURPLE}${problem}${NC}"
  $compiler ${def_fn} ${tmp_exec} 2> ${tmp_log}

  if [ $? -ne 0 ]; then
    echo "Problem ${problem} could not be compiled"
    cat ${tmp_log}
    exit 1
  fi

  truncate -s 0 ${tmp_log}

  if [ "$1" == "-" ]; then
    timeout --preserve-status 1s $simulator -lstack 10241024 -q -f ${tmp_exec} > ${tmp_out} 2> ${tmp_log}
  else
    echo "Running MIPS program"
    time sh -c "timeout --preserve-status 1s $simulator -lstack 10241024 -q -f ${tmp_exec} > ${tmp_out} 2> ${tmp_log}"
  fi


  if [ $? -ne 0 ]; then
    echo -e "${CYAN}TLE or maybe RTE:${NC} your program for ${problem} exited with non-zero status"
    echo;
    continue
  fi

  if [ -s "$tmp_log" ]; then
    echo -e "${CYAN}RTE:${NC} SPIM printed to stderr, something went wrong"
    cat ${tmp_log}
    exit 1
  fi

  if grep -q Exception "${tmp_out}"; then
    echo -e "${CYAN}RTE:${NC} exceptions were found in your output file. please check it (cat ${tmp_out})"
    cat ${tmp_log}
    exit 1
  fi

  tail -n +6 ${tmp_out} > ${tmp_log} && mv ${tmp_log} ${tmp_out}
  cp ${tmp_out} ${yours_fn}

  diff ${output_fn} ${tmp_out}

  if [ $? -ne 0 ]; then
    echo -e "${RED}WA:${NC} found differences between your answer and
      the expected answer"

    #exit 1
  else
    echo -e "${GREEN}AC:${NC} outputs matched"
  fi

  echo;
done
