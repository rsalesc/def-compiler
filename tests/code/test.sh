#!/bin/bash

compiler=../../a.out
tmp_code=/tmp/cdef_code
tmp_out=/tmp/cdef_out
tmp_exec=/tmp/cdef_exec
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

  echo "- Compiling solution for ${problem}"
  $compiler ${def_fn} ${tmp_exec} 2> ${tmp_log}

  if [ $? -ne 0 ]; then
    echo "Problem ${problem} could not be compiled"
    cat ${tmp_log}
    exit 1
  fi

  truncate -s 0 ${tmp_log}

  echo "Running MIPS program"
  timeout --preserve-status 1s $simulator -lstack 10241024 -q -f ${tmp_exec} > ${tmp_out} 2> ${tmp_log}

  if [ $? -ne 0 ]; then
    echo -e "TLE or maybe RTE: your program for ${problem} exited with non-zero status"
    echo;
    continue
  fi

  if [ -s "$tmp_log" ]; then
    echo -e "RTE: SPIM printed to stderr, something went wrong"
    cat ${tmp_log}
    exit 1
  fi

  if grep -q Exception "${tmp_out}"; then
    echo -e "RTE: exceptions were found in your output file. please check it (cat ${tmp_out})"
    cat ${tmp_log}
    echo;
    continue
  fi

  tail -n +6 ${tmp_out} > ${tmp_log} && mv ${tmp_log} ${tmp_out}

  diff ${output_fn} ${tmp_out}

  if [ $? -ne 0 ]; then
    echo -e "WA: found differences between your answer and 
      the expected answer"

    #exit 1
  else
    echo -e "AC: outputs matched"
  fi

  echo;
done
