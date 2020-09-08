#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
#include <string>

using namespace std;

enum TokenType
{
  identifier,
  newline,
  op,
  keyword,
  body,
  separator,
  number,
  boolean,
  str
};
static const char *TokenTypeString[] = {
    "identifier",
    "newline",
    "op",
    "keyword",
    "body",
    "separator",
    "number",
    "boolean",
    "str"};

enum TokenAttr
{
  none,
  start,
  end,
  decimal,
  hexidecimal
};

static const char *TokenAttrString[] = {
    "none",
    "start",
    "end",
    "decimal",
    "hexidecimal"};

class Token
{
private:
  TokenType m_type;
  string m_value;
  TokenAttr m_attr;

public:
  Token(TokenType type, string value)
  {
    m_type = type;
    if (m_type == TokenType::newline)
      m_value = ';';
    else
      m_value = value;
    m_attr = TokenAttr::none;
  }
  Token(TokenType type, string value, TokenAttr attr) : Token(type, value)
  {
    m_attr = attr;
  }

  friend ostream &operator<<(ostream &out, const Token &t)
  {
    out << "(" << TokenTypeString[t.m_type];
    if (t.m_attr != 0)
      out << "-" << TokenAttrString[t.m_attr];
    out << "," << t.m_value << ")";
    return out;
  }
};

#endif