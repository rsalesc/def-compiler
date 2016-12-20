#include "stream.hpp"

char Stream::peek(){
  return s.peek();
}

char Stream::get(){
  char c = s.get();
  col++;
  if(c == '\n'){
    cols.push_back(col);
    line++;
    col = 0;
  }

  return c;
}

void Stream::unget(){
  s.unget();

  if(--col < 0){
    line--;
    col = cols.back()-1;
    cols.pop_back();
  }
}

std::pair<int, int> Stream::location(){
  return {line, col};
}
