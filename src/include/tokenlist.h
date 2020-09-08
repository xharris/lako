#ifndef TOKENLIST_H
#define TOKENLIST_H

#include "token.h"

#include <vector>
#include <string>

using namespace std;

class TokenList
{
private:
  vector<Token> token_list;

public:
  void addToken(Token);
  void tokenizeFile(const char *);
};

#endif