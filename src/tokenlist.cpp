#include "include/tokenlist.h"

#include <fstream>
#include <sstream>
#include <cstring>
#include <iostream>
#include <cctype>
#include <vector>

using namespace std;

const bool LOG_COMMENTS = false;
const char NOT_INSIDE = 0;

enum Comment
{
  no_comment,
  single,
  multi
};
enum Number
{
  nan,
  dec,
  hex
};

int isNewLine(int c)
{
  return c == ';' || (iscntrl(c) && isspace(c));
}

bool validInIdentifier(int c, bool first_char)
{
  return first_char ? c == '_' || c == '$' || isalpha(c) : c == '_' || isalnum(c);
}

bool isOperator(int c)
{
  return c == '+' || c == '-' || c == '/' || c == '*' || c == '&' ||
         c == '|' || c == '=' || c == '<' || c == '>' || c == '.' ||
         c == ':' || c == '!';
}

bool isBody(int c)
{
  return c == '(' || c == ')' || c == '{' || c == '}';
}

int isBodyStart(int c)
{
  if (c == '(')
    return 1;
  if (c == '{')
    return 2;
  return 0;
}

int isBodyEnd(int c)
{
  if (c == ')')
    return 1;
  if (c == '}')
    return 2;
  return 0;
}

bool isStringStartEnd(int c)
{
  return c == '\'' || c == '\"' || c == '`';
}

const int KEYWORDS_LEN = 17;
const char *KEYWORDS[] = {
    "var", "const", "if", "for", "while",
    "do", "number", "string", "bool", "array",
    "map", "is", "true", "false", "return",
    "self", "new"};

bool isKeyword(string token)
{
  if (strcmp(token.c_str(), "$") == 0)
    return true;
  for (int k = KEYWORDS_LEN; k--;)
  {
    if (strcmp(token.c_str(), KEYWORDS[k]) == 0)
      return true;
  }
  return false;
}

bool isSeparator(int c)
{
  return c == ',';
}

void TokenList::tokenizeFile(const char *filename)
{
  // load script
  ifstream f_script(filename, ifstream::in);
  char c = f_script.get();
  char c_next = f_script.peek();

  stringstream token_stream;
  int parsing_token = 0;

  // simple token checks
  bool is_whole_token, is_new_line, is_identifier, is_operator, is_body, is_string_startend, is_escaping, is_separator;
  // complex token checks
  char inside_string = NOT_INSIDE;
  bool inside_body;
  int last_body;
  bool inside_comment;
  Number is_number = Number::nan;
  bool number_has_dec;
  // token stacks
  vector<int> body_stack;
  vector<Comment> comment_stack;

  while (f_script.good())
  {
    c_next = f_script.peek();

    // inside {} or ()?
    inside_body = body_stack.size() > 0;

    // starting comment
    if ((!inside_string || inside_body) && c == '/' && c_next == '*')
    {
      if (LOG_COMMENTS)
        cout << "(comment start multi)\n";
      comment_stack.push_back(Comment::multi);
    }
    if (!inside_string && c == '/' && c_next == '/')
    {
      if (LOG_COMMENTS)
        cout << "(comment start single)\n";
      comment_stack.push_back(Comment::single);
    }

    inside_comment = comment_stack.size() > 0;

    if (parsing_token == 0 && inside_comment == Comment::no_comment && (!inside_string || inside_body))
    {
      is_identifier = validInIdentifier(c, true);

      // starting dec/hex number?
      if (is_number == Number::nan)
      {
        number_has_dec = false;
        if (c == '0' && c_next == 'x')
          is_number = Number::hex;
        else if (isdigit(c))
          is_number = Number::dec;
      }
    }
    is_new_line = isNewLine(c);
    is_operator = isOperator(c);
    is_body = isBody(c);
    is_string_startend = isStringStartEnd(c);
    is_separator = isSeparator(c);
    is_escaping = !is_escaping && inside_string != NOT_INSIDE && c == '\\';

    is_whole_token = (is_identifier && !validInIdentifier(c_next, false)) ||
                     (is_operator && !is_number) ||
                     isBody(c_next) ||
                     is_body ||
                     is_string_startend ||
                     isStringStartEnd(c_next) ||
                     is_new_line ||
                     isNewLine(c_next) ||
                     is_separator;

    // finished decimal/hex number?
    if (parsing_token > 0 && is_number != Number::nan)
    {
      if (c_next == '.')
      {
        if (number_has_dec)
          ; // error: number can only have one decimal point
        else if (is_number == Number::hex)
          ; // error: hex cannot have decimal point
        else
          number_has_dec = true;
      }

      if (is_number == Number::dec && !isdigit(c_next) && c_next != '.')
        is_whole_token = true;

      if (is_number == Number::hex && !isxdigit(c_next))
        is_whole_token = true;
    }

    if (!isspace(c) || isNewLine(c) || (inside_string && !inside_body))
    {
      parsing_token++;
      token_stream << c;
    }

    if (is_whole_token)
    {
      if (parsing_token > 0 && inside_comment == Comment::no_comment)
      {
        // ignore these tokens unless it's outside a string OR inside a block of string interpolation
        if (!inside_string || inside_body)
        {
          if (is_identifier)
          {
            addToken(Token(isKeyword(token_stream.str()) ? TokenType::keyword : TokenType::identifier, token_stream.str()));
            is_identifier = false;
          }
          if (is_new_line)
            addToken(Token(TokenType::newline, token_stream.str()));
          if (is_operator)
            addToken(Token(TokenType::op, token_stream.str()));
          if (is_number != Number::nan)
          {
            addToken(Token(TokenType::number, token_stream.str()));
            is_number = Number::nan;
          }
          if (is_separator)
            addToken(Token(TokenType::separator, token_stream.str()));
        }
        // character in a string
        else if (!is_string_startend && !is_body)
        {
          addToken(Token(TokenType::str, token_stream.str()));
        }
        if (is_body && !is_escaping)
        {
          if (isBodyStart(c))
            body_stack.push_back(isBodyStart(c));

          if (isBodyEnd(c) && inside_body)
          {
            last_body = body_stack.back();
            if (last_body != isBodyEnd(c))
              ; // throw error: mismatch body start/end

            body_stack.pop_back();
          }

          addToken(Token(TokenType::body, token_stream.str(), isBodyStart(c) ? TokenAttr::start : TokenAttr::end));
        }
        if (is_string_startend)
        {
          // check if string starter matches string ender
          if (inside_string != NOT_INSIDE && !is_escaping && c != inside_string)
          {
            // throw error
          }
          else
          {
            addToken(Token(TokenType::str, token_stream.str(), inside_string == c ? TokenAttr::end : TokenAttr::start));
            inside_string = inside_string == c ? NOT_INSIDE : c;
          }
        }
      }

      token_stream.str(std::string());
      parsing_token = 0;
    }

    // uncomment
    if (inside_comment)
    {
      if (comment_stack.back() == Comment::single && ((is_new_line && c != ';') || (c == '*' && c_next == '/')))
      {
        if (LOG_COMMENTS)
          cout << "(comment end)\n";
        comment_stack.pop_back();
      }
      if (comment_stack.back() == Comment::multi && c == '*' && c_next == '/')
      {
        if (LOG_COMMENTS)
          cout << "(comment end)\n";
        comment_stack.pop_back();
        f_script.get();
        token_stream.str(std::string());
      }
    }

    c = f_script.get();
  }
  f_script.close();
}

void TokenList::addToken(Token token)
{
  token_list.push_back(token);
  cout << token << '\n';
}