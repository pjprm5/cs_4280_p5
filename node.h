// Paul Passiglia
// cs_4280 P4 - Code Generation
// node.h

#ifndef NODE_H
#define NODE_H
#include <set>
#include <vector>
#include "token.h"
#include "semStack.h"

template <class T>
class node {
  private:
      std::vector<node<T>> children_;
      std::vector<token> tokens_;

      T key_;
      void preHelper(std::ostream& out, std::string indent);

      void statChecker(stack<std::string, int> &stat, int scope);

      void genASM(std::ostream& out, int scope, std::set<std::string>& varset, stack<std::string, int>& stackvars, std::string& labelctr);

      void genChildASM(std::ostream& out, int scope, std::set<std::string>& varset, stack<std::string, int>& stackvars, std::string& labelctr);

      void getNextLabelString(std::string& labelctr);

      void setRelOpCall(std::ostream& out, int scope, std::set<std::string>& varset, stack<std::string, int>& stackvars, std::string& labelctr, std::string& endLabel);

  public:
      // AST functionality
      void insert(node<T> child);
      void insert(token childToken);
      node<T> (T key);
      node<T> ();

      // traversal functionality
      void preOrder(std::ostream& out);
      void preOrder();
      std::string toString();

      //Enables Static Semantics
      void statChecker();

      void genASM(std::ostream& out);

      std::string endLabelHolder = "";
      std::string startLabelHolder = "";
};

template <class T>
node<T>::node()
{

}

template <class T>
node<T>::node(T key)
{
  key_ = key; // Initialize the node name
}

template <class T>
void node<T>::insert(node<T> child)
{
  children_.push_back(child);
}

template <class T>
void node<T>::insert(token childToken)
{
  tokens_.push_back(childToken);
}

template <class T>
std::string node<T>::toString()
{
if (tokens_.size() > 0)
  {
    std::string tokenStr("");
    int i = 0;
    while (i < tokens_.size()-1)
    {
      tokenStr += tokens_[i].toString() + ", ";
      i++;
    }
    tokenStr += tokens_[i].toString() + "\n";
    return key_ + "  Tokens: " + tokenStr;
  }
  return key_ + "\n";
}

template <class T>
void node<T>::preHelper(std::ostream& out, std::string indent)
{
  // helper for preOrder
  std::string newdent("  ");
  newdent += indent;
  out << indent << toString();
  for (int i = 0; i < children_.size(); i++)
  {
      children_[i].preHelper(out, newdent);
  }
}

// traverse with variable ostream
template <class T>
void node<T>::preOrder(std::ostream& out)
{
  preHelper(out, "");
}

// traversal cout stream
template <class T>
void node<T>::preOrder()
{
  preHelper(std::cout, "");
}

template <class T>
void node<T>::statChecker()
{
  stack<std::string, int> s;

  statChecker(s, 0);
}

template <class T>
void node<T>::statChecker(stack<std::string, int> &stat, int scope)
{
  //std::cout << "In statChecker for node: " << key_ << "\n";
  if(key_ == "<vars>" && tokens_.size())
  {
    //std::cout << "token value is: " << tokens_[1].instance << "\n"; 
    std::string k = tokens_[1].instance;
    //std::cout << "k value is: " << k << "\n";

    if(stat.containsKey(k, scope))
    {
      std::cout << "SEMANTICS ERR\n";
      std::cout << "Line " << tokens_[1].posString() << " \"";
      std::cout << tokens_[1].instance;
      std::cout << "\" has already been defined in this scope\n";

      exit(-3);
    }

    stat.push(k, scope);

  }
  else if(key_ == "<block>")
  {
    for(int i = 0; i < children_.size(); i++)
    {
      children_[i].statChecker(stat, scope + 1);
    }

    stat.popScope(scope + 1);
    return;
  }
  else if(key_ == "<R>" || key_ == "<inbound>" || key_ == "<assign>" || key_ == "<label>" || key_ == "<jump>")
  {
    for(int i = 0; i < tokens_.size(); i++)
    {
      if(tokens_[i].id == identifier)
      {
        std::string k = tokens_[i].instance;

        if(!stat.containsKey(k))
        {
          std::cout << "SEMANTICS ERR\n";
          std::cout << "Line " << tokens_[i].posString() << " \"";
          std::cout << tokens_[i].instance;
          std::cout << "\" has not been defined in this scope\n";
          std::cout << "tokens that are in vector are: " << "\n";
          exit(-3);
        }
      }
    }
  }

  for(int i = 0; i < children_.size(); i++)
  {
    children_[i].statChecker(stat, scope);
  }

}

template <class T>
void node<T>::genASM(std::ostream& out)
{
  std::set<std::string> varset;
  stack<std::string, int> stackvars;
  std::string labelctr("A");

  genASM(out, 0, varset, stackvars, labelctr);
}

template <class T>
void node<T>::genASM(std::ostream& out, int scope, std::set<std::string>& varset, stack<std::string, int>& stackvars, std::string& labelctr)
{
  bool verbose = false;

  if (key_ == "<program>")
  {         // vars block
    genChildASM(out, scope, varset, stackvars, labelctr);
    out << "STOP\n";
    std::set<std::string>::iterator itr;

    for (itr = varset.begin(); itr != varset.end(); ++itr)
    {
      out << *itr << " 0\n";
    }

    //these will be our temp variables
    out << "printV 0\n";
    out << "calcV 0\n";
  }
  else if (key_ == "<block>")
  {         // start vars stats stop
    genChildASM(out, scope+1, varset, stackvars, labelctr);
    // restore variables stacked within block
    while (stackvars.getLastScope() == scope+1)
    {
      out << "STACKR 0\n";
      out << "STORE " << stackvars.getLastKey() << "\n";
      out << "POP\n";
      //out << "test block";
      stackvars.pop();
    }
  }
  else if (key_ == "<vars>")
  {        
    if (tokens_.size())
    {
      if (varset.find(tokens_[1].instance) != varset.end())
      {
        // variable needs to be pushed to the stack
        out << "PUSH\n";
        out << "LOAD " << tokens_[1].instance << "\n";
        out << "STACKW 0\n";
        stackvars.push(tokens_[1].instance, scope);
      }

      // mult by -1 for unary operator
      if(tokens_[3].instance == "-") 
      {
        out << "LOAD " << tokens_[4].instance << "\n";
        out << "MULT -1\n";
        out << "STORE " << tokens_[1].instance << "\n";
      }
      else
      {
        out << "LOAD " << tokens_[3].instance << "\n";
        out << "STORE " << tokens_[1].instance << "\n";
      }

      varset.insert(tokens_[1].instance);
    }

    genChildASM(out, scope, varset, stackvars, labelctr);
  }
  else if (key_ == "<expr>")
  {                         // N - expr | N
    genChildASM(out, scope, varset, stackvars, labelctr);

    if(tokens_.size())
    {
      out << "STACKR 0\n";
      out << "POP\n";
      out << "STORE calcV\n";
      out << "STACKR 0\n";
      out << "SUB calcV\n";
      out << "STACKW 0\n";
    }
  }
  else if (key_ == "<A>")
  {                         // M + A | M
    genChildASM(out, scope, varset, stackvars, labelctr);

    if(tokens_.size())
    {
      out << "STACKR 0\n";
      out << "POP\n";
      out << "STORE calcV\n";
      out << "STACKR 0\n";
      out << "ADD calcV\n";
      out << "STACKW 0\n";
    }
  }
  else if (key_ == "<N>")
  {                         // A / N | A * N | A
    genChildASM(out, scope, varset, stackvars, labelctr);

    if(tokens_.size() && tokens_[0].instance == "*")
    {
      out << "STACKR 0\n";
      out << "POP\n";
      out << "STORE calcV\n";
      out << "STACKR 0\n";
      out << "MULT calcV\n";
      out << "STACKW 0\n";
    }
    else if (tokens_.size() && tokens_[0].instance == "/")
    {
      out << "STACKR 0\n";
      out << "POP\n";
      out << "STORE calcV\n";
      out << "STACKR 0\n";
      out << "DIV calcV\n";
      out << "STACKW 0\n";
    }
  }
  else if (key_ == "<M>")
  {                         // * M | R
    genChildASM(out, scope, varset, stackvars, labelctr);
    if (tokens_.size() == 1)
    {
      out << "STACKR 0\n";
      out << "MULT -1\n";
      out << "STACKW 0\n";
    }
  }
  else if (key_ == "<R>")
  {                         // (expr) | id_tk | num_tk
    genChildASM(out, scope, varset, stackvars, labelctr);

    // (expr) is on stack
    if (tokens_[0].id == identifier || tokens_[0].id == integer)
    {
      out << "PUSH\n";
      out << "LOAD " << tokens_[0].instance << "\n";
      out << "STACKW 0\n";
    }
  }
  else if (key_ == "<stats>")
  {         // stat mstat
    genChildASM(out, scope, varset, stackvars, labelctr);
  }
  else if (key_ == "<mStat>")
  {         // empty | stat mstat
    genChildASM(out, scope, varset, stackvars, labelctr);
  }
  else if (key_ == "<stat>")
  {         // inbound ;| outbound ;| block ;| if ;| loop ;| assign ;| jump ;| label;
    genChildASM(out, scope, varset, stackvars, labelctr);
  }
  else if (key_ == "<inbound>")
  {         // listen id_tk
    out << "READ " << tokens_[1].instance << "\n";
  }
  else if (key_ == "<outbound>")
  {         // talk expr
    genChildASM(out, scope, varset, stackvars, labelctr);
    out << "STACKR 0\n";
    out << "POP\n";
    out << "STORE printV\n";
    out << "WRITE printV\n";
  }
  else if (key_ == "<if>")
  {
    // if [ exprA RelOp exprB ] then stat
    // no asm for stat yet.
    for (int i = 0; i < children_.size()-1; i++)
    {
      children_[i].genASM(out, scope, varset, stackvars, labelctr);
    }

    // exprB -> stack[0]
    // exprA -> stack[1]
    out << "STACKR 0\n";
    out << "POP\n";
    out << "STORE calcV\n";
    out << "STACKR 0\n";
    out << "POP\n";
    out << "SUB calcV\n";
    // ACC: a-b

    setRelOpCall(out, scope, varset, stackvars, labelctr, labelctr);

    //out << labelctr << "\n";
    std::string oldLabel(labelctr.c_str());
    getNextLabelString(labelctr);

    // gen asm for stat
    children_[children_.size()-1].genASM(out, scope, varset, stackvars, labelctr);
    out << oldLabel << ": NOOP\n";
  }
 
  else if (key_ == "<loop>")
  {         // loop [ expr RelOp expr ] stat

      out << labelctr << ": NOOP\n";
      std::string startLabel(labelctr.c_str());
      getNextLabelString(labelctr);
      std::string endLabel(labelctr.c_str());
      getNextLabelString(labelctr);

      // is condition true?
      for (int i = 0; i < children_.size()-1; i++)
      {
        children_[i].genASM(out, scope, varset, stackvars, labelctr);
      }
      // exprb -> stack[0]
      // expra -> stack[1]
      out << "STACKR 0\n";
      out << "POP\n";
      out << "STORE calcV\n";
      out << "STACKR 0\n";
      out << "POP\n";
      // ACC: a-b
      setRelOpCall(out, scope, varset, stackvars, labelctr, endLabel);

      //gen asm for stat
      children_[children_.size()-1].genASM(out, scope, varset, stackvars, labelctr);

      out << "BR " << startLabel << "\n";
      out << endLabel << ": NOOP\n";

  }
  else if (key_ == "<assign>")
  {         // assign id_tok = expr
    genChildASM(out, scope, varset, stackvars, labelctr);
    out << "STACKR 0\n";
    out << "POP\n";
    out << "STORE " << tokens_[1].instance << "\n";
  }
  else if(key_ == "<label>")
  {
    out << tokens_[1].instance << ": NOOP\n";
  }
  else if(key_ == "<jump>") 
  {
    out << "BR " << tokens_[1].instance << "\n";
  }
}


template <class T>
void node<T>::genChildASM(std::ostream& out, int scope, std::set<std::string>& varset, stack<std::string, int>& stackvars, std::string& labelctr)
{
  for (int i = 0; i < children_.size(); i++)
  {
    children_[i].genASM(out, scope, varset, stackvars, labelctr);
  }
}

template <class T>
void node<T>::getNextLabelString(std::string& labelctr)
{
  int lasti = labelctr.length()-1;

  if ((int)labelctr[lasti] < 122)
  {
    labelctr[lasti]++;
  }
  else
  {
    if ((int)labelctr[0] == 122)
    {
      labelctr.append("A");
    }

    for (int i = lasti; i >= 0; i--)
    {
      if ((int)labelctr[i] >= 122)
      {
        labelctr[i] = 'A';
        if (i > 0) labelctr[i-1]++;
      }
      else
      {
        break;
      }
    }
  }
}

// RelOp decides which op_tk to use based on that reading
template <class T>
void node<T>::setRelOpCall(std::ostream& out, int scope, std::set<std::string>& varset, stack<std::string, int>& stackvars, std::string& labelctr, std::string& endLabel)
{
  std::string tempString = tokens_[2].instance;
 
  if (tempString == "<")
  {
    //out << "test1";
    out << "SUB calcV\n";
    out << "BRPOS " << endLabel << "\n";
  }
  else if (tempString == ">")
  {
    //out << "test2";
    out << "SUB calcV\n";
    out << "BRNEG " << endLabel << "\n";
  }
  else if(tempString == "==")
  {
    //out << "test3";
    out << "SUB calcV\n";
    out << "BRPOS " << endLabel << "\n";
    out << "BRNEG " << endLabel << "\n";
  }
  else
  {
    //out << "test4";
    out << "SUB calcV\n";
    out << "BRZERO " << endLabel << "\n";
  }
}
#endif
