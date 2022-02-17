// Paul Passiglia
// cs_4280 P4 - Code Generation
// scanner.cpp

#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include "token.h"
#include "fsa.h"


using namespace std;

static int lineNum = 1;
static int charNum = 1;

tokenID filterKeyword(std::string cap)
{
  
  if (cap == "start") return keyword;
  if (cap == "stop") return keyword;
  if (cap == "loop") return keyword;
  if (cap == "while") return keyword;
  if (cap == "for") return keyword;
  if (cap == "label") return keyword;
  if (cap == "exit") return keyword;
  if (cap == "listen") return keyword;
  if (cap == "talk") return keyword;
  if (cap == "program") return keyword;
  if (cap == "if") return keyword;
  if (cap == "then") return keyword;
  if (cap == "assign") return keyword;
  if (cap == "declare") return keyword;
  if (cap == "jump") return keyword;
  if (cap == "else") return keyword;
  
  return identifier;
}

token scan(std::istream& in)
{
    int linePos = lineNum;
    int charPos = charNum;
    int lineSize = 0;
    int charSize = 0;
    int state = 1;
    char c = char(32);
    std::string capture("");

    while(state > 0 && state <= 1000)
    {
        in.get(c);
        //cout << (int)c << in.fail() << "\t" << linePos << ":" << charPos << "\n"; // positions
        if(in.eof() || in.fail())
        {
            c = (char)-1;
        }

        if(state==1)
        {
            if (c > 32 && c != 38) 
            {
                //Is not leading whitespace, capture this (except &)
                capture += std::string(1, c);
                charSize++;
            }
            else if (c == 38) 
            {
                //set token to start at beginning of next line
                linePos++;
                charPos = 1;
                lineSize++;
                charSize = 1-charNum;
            }
            else if (c == 32 || c == 9)
            {
                //shift forward start of token index
                charPos++;
                charSize++;
            }
            else if (c == 10)
            {
                //shift down start of token index
                linePos++;
                charPos = 1;
                lineSize++;
                charSize = 1-charNum;
            }
        }
        else if (state > 1 && state < 11)
        {
            capture += std::string(1, c);
            charSize++;
        }

        state = fsa[state][(int)c];
    }

    in.unget();
    charSize--;

    std::string e(1,c);
    capture = capture.substr(0,capture.length()-1);
    std::pair<int, int> pos = std::make_pair(linePos,charPos);
    std::pair<int, int> size = std::make_pair(lineSize, charSize);
    std::pair<int, int> erpos = std::make_pair(lineNum+lineSize, charNum+charSize);

    token t = token((tokenID)state, capture, pos, size);

    switch(state)
    {
        case -1:
          t = token(error, "no valid token begins with '" + e + "'", erpos, size);
          cout << t.toString() << "\n";
          exit(-1);
          break;

        case 0:
          t = token(error, "illegal character '" + e + "'", erpos, size);
          cout << t.toString() << "\n";
          exit(-1);
          break;

        case 1001:
          t = token(filterKeyword(capture), capture, pos, size);
          break;

        case 1005:
          t = token(eoftk, "EOF", pos, size);
          break;
    }

    lineNum += std::get<0>(t.size);
    charNum += std::get<1>(t.size);

    return t;
}
