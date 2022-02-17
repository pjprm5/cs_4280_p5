// Paul Passiglia
// cs_4280 P4 - Code Generation
// main.cpp


#include <iostream>
#include <string>
#include <fstream>
#include "parser.h"
#include "node.h"

using namespace std;

//main prints out the tree for parsing
int main(int argc, char* argv[])
{
  std:: string base;
  node<std::string> root;

  if(argc == 1)
  {
    root = parser(cin);
    base = "kb";
  }
  else if(argc == 2) //check to see if the file has the proper extension
  {
    string fext, filename(""), filearg(argv[1]);
    int length = filearg.length();

    if(length > 2)
    {
      fext = filearg.substr(length-3, length);

      if(fext == ".fs")
      {
        filename = filearg;
      }
    }

    if(filename == "")
    {
      filename = filearg + ".fs";
    }

    ifstream fs(filename.c_str());

    if(fs)
    {
      root = parser(fs);
      base = filename.substr(0, filename.length()-3); // check
    }
    else
    {
      cout << filename << ": cannot be opened.";
      return 1;
    }
  }
  else
  {
    cout << "Invalid # of arguments given.";
    return 2;
  }

  root.statChecker();

  std::ofstream ofs;
  ofs.open(base + ".asm");
  root.genASM(ofs);
  ofs.close();

  cout << "Program finished: the asm file is located: " << base << ".asm" << endl;
  return 0;
}
