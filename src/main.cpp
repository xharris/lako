#define _GLIBCXX_USE_CXX11_ABI 0
#include "include/tokenlist.h"
#include "include/parser.h"

using namespace std;

void log(const char *output)
{
  cout << "LOG:" << output << "\n";
}

int err(const char *output)
{
  cerr << output << "\n";
  return 0;
}

int main(int argc, char *argv[])
{
  log("Program start");

  if (argc == 1)
  {
    return err("No script given.");
  }

  // lexing tokens from file
  TokenList token_list;
  token_list.tokenizeFile(argv[1]);

  // parse tokens

  log("Program end...");
  return 0;
}