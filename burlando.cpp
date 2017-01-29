#include <bits/stdc++.h>
#include "lexer/common.hpp"
#include "lexer/dfa.cpp"
#include "lexer/nfa.cpp"
#include "lexer/token.hpp"
#include "lexer/regex.cpp"

using namespace std;

string unpoint(const string & x){
  string res;
  for(char c : x) if(c != '.') res += c;
  return res;
}

int main(){
  ios::sync_with_stdio(false);
  cin.tie(0);

  string R;
  while(cin >> R){
    Regex re(unpoint(R));
    int P;
    cin >> P;

    getline(cin, R);
    while(P--){
      getline(cin, R);

      cout << (re.run(R) ? "Y" : "N") << endl;
    }

    cout << endl;
  }
}
