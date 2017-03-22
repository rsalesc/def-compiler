#!/bin/bash

for problem_folder in *.p/; do
  problem=${problem_folder/\.p\//}
  output_fn="${problem}.p/out"
  def_fn="${problem}.p/sol.def"
  code_fn="${problem}.p/sol.c"

  echo "- Processing problem ${problem}..."
  echo "Generating equivalent C code"
  ./cdef < ${def_fn} > ${code_fn}

  echo "Compiling C code"
  gcc ${code_fn}

  echo "Running C code and generating output"
  ./a.out > ${output_fn}
done
