// Paul Passiglia
// cs_4280 P4 - Code Generation
// parser.cpp

#include <iostream>
#include <string>
#include <fstream>
#include "scanner.h"
#include "token.h"
#include "node.h"

using namespace std;

static token t;
static std::istream in(NULL);
static void parseErr(std::string expected);

static node<std::string> Program();
static node<std::string> Block();
static node<std::string> Vars();
static node<std::string> Expr();
static node<std::string> N();
static node<std::string> A();
static node<std::string> M();
static node<std::string> R();
static node<std::string> Stats();
static node<std::string> mStat();
static node<std::string> Stat();
static node<std::string> Inbound();
static node<std::string> Outbound();
static node<std::string> If();
static node<std::string> Loop();
static node<std::string> Assign();
static node<std::string> RelOp();
static void RelOp(node<std::string>& root); 
static node<std::string> Label();
static node<std::string> Jump();

node<std::string> parser(std::istream& stream)
{
  in.rdbuf(stream.rdbuf());
  t = scan(in);

  node<std::string> root = Program();

  if(t.id != eoftk)
  {
    parseErr("EOFTK");
  }

  return root;
}

//<program> -> <vars> kw_tok(program) <block>
static node<std::string> Program()
{
  node<std::string> root("<program>"); //set the root of the node to <program>

  root.insert(Vars()); 

  //check to see if the tokenID is a keyword
  if(t.id == keyword && !t.instance.compare("program")) 
  {
    root.insert(t);
    t = scan(in); 
    root.insert(Block()); 
  }

  return root; //return the root vector
}

//<block> -> kw_tok(start) <vars> <stats> kw_tok(stop)
static node<std::string> Block()
{
  node<std::string> root("<block>"); //set the root of the node to <block>

  //check to see if the tokenID is a kw_tok(begin)
  if(t.id == keyword && !t.instance.compare("start")) 
  {
    root.insert(t);
    t = scan(in);
    root.insert(Vars());
    root.insert(Stats());

    //check now to see if the <block> definition is at the last terminal
    if(t.id == keyword && !t.instance.compare("stop")) 
    {
      root.insert(t); 
      t = scan(in); 
      return root; 
    }
    parseErr("kw_tok: 'stop'"); 
  }
  parseErr("kw_tok: 'start'");
  return root; 
}

//<vars> -> Ɛ | kw_tok(data) id_tok(identifier) op_tok(:) op_tok(=) num_tok(integer) op_tok(;) <vars>
static node<std::string> Vars()
{
  node<std::string> root("<vars>"); //set the root of the node to <vars>

  //check to see if tokenID is a kw_tok("declare")
  if(t.id == keyword && !t.instance.compare("declare")) 
  {
    root.insert(t); 
    t = scan(in); 

    //if tokenID is any tkID
    if(t.id == identifier)
    {
      root.insert(t); 
      t = scan(in); 

      //if tokenID is op_tok(":")
     /* if(t.id == opordel && !t.instance.compare(":"))
      {
        root.insert(t); //insert root into node
        t = scan(in); //scan for next token*/

        //if tokenID is op_tok("=")
        if(t.id == opordel && !t.instance.compare("="))
        {
          root.insert(t); 
          t = scan(in); 

          //if tokenID is any num_tok
          if(t.id == integer)
          {
            root.insert(t);
            t = scan(in); 
            root.insert(Vars()); 

            //if tokenID is op_tok(";")
            if(t.id == opordel && !t.instance.compare(";"))
            {
              root.insert(t);
              t = scan(in);

              // catchall
              if(t.instance == "declare")
              {
                Vars();
              }
              return root;
            }
            return root;
          }
          parseErr("num_tok");
        }
        parseErr("op_tok: '='");
     /* }
      parseErr("op_tok: ':'");*/
    }
    parseErr("id_tok");
  }
  return root;
}

//<expr> -> <N> - <expr> | <N>
static node<std::string> Expr()
{
  node<std::string> root("<expr>");

  root.insert(N());

  if(t.id == opordel && !t.instance.compare("-"))
  {
    root.insert(t);
    t = scan(in);
    root.insert(Expr());
    return root;
  }

  return root;
}

//<N> -> <A> / <N> | <A> * <N> | <A>
static node<std::string> N()
{
  node<std::string> root("<N>");

  root.insert(A());

  if(t.id == opordel && !t.instance.compare("/"))
  {
    root.insert(t);
    t = scan(in);
    root.insert(N());
    return root;
  }
  else if(t.id == opordel && !t.instance.compare("*"))
  {
    root.insert(t);
    t = scan(in);
    root.insert(N());
    return root;
  }
  return root;
}

//<A> -> <M> + <A> | <M>
static node<std::string> A()
{
  node<std::string> root("<A>");

  root.insert(M());

  if(t.id == opordel && !t.instance.compare("+"))
  {
    root.insert(t);
    t = scan(in);
    root.insert(A());
    return root;
  }

  return root;
}

// <M> -> * <M> | <R>
static node<std::string> M()
{
  node<std::string> root("<M>");

  if(t.id == opordel && !t.instance.compare("*"))
  {
    root.insert(t);
    t = scan(in);
    root.insert(M());
    return root;
  }
  else
  {
    root.insert(R());
    return root;
  }

  return root;
}

//<R> -> op_tok(() <expr> op_tok()) | id_tok | num_tok
static node<std::string> R()
{
  node<std::string> root("<R>");

  if(t.id == opordel && !t.instance.compare("("))
  {
    root.insert(t);
    t = scan(in);
    root.insert(Expr());

    if(t.id == opordel && !t.instance.compare(")"))
    {
      root.insert(t);
      t = scan(in);
      return root;
    }
    parseErr("op_tok: ')'");
  }
  else if(t.id == identifier)
  {
    root.insert(t);
    t = scan(in);
    return root;
  }
  else if(t.id == integer)
  {
    root.insert(t);
    t = scan(in);
    return root;
  }
  parseErr("id_tok or num_tok");
  return root;
}

//<stats> -> <stat> <mStat>
static node<std::string> Stats()
{
  node<std::string> root("<stats>");

  root.insert(Stat());
  root.insert(mStat());

  return root;
}

//<mStat> -> Ɛ | <stat> <mStat>
static node<std::string> mStat()
{
  node<std::string> root("<mStat>");

  if(t.id == keyword && (!t.instance.compare("if") || !t.instance.compare("while") || 
       !t.instance.compare("stat") || !t.instance.compare("talk") || !t.instance.compare("listen") ||
       !t.instance.compare("assign") || !t.instance.compare("label") ||
       !t.instance.compare("jump") || !t.instance.compare("start")))
  {
    root.insert(Stat());
    root.insert(mStat());

    return root;
  }

  return root;
}

/* <stat> -> <inbound> op_tok(;) | <outbound> op_tok(;) | <block> |
 * <if> op_tok(;) | <loop> op_tok(;) | <assign> op_tok(;) |
 * <jump> op_tok(;) | <label> op_tok(;) |
 */
static node<std::string> Stat()
{
  node<std::string> root("<stat>");

  if(t.id == keyword && !t.instance.compare("listen"))
  {
    root.insert(Inbound());
    if(t.id == opordel && !t.instance.compare(";"))
    {
      root.insert(t);
      t = scan(in);
      return root;
    }
    return root;
  }
  else if(t.id == keyword && !t.instance.compare("talk"))
  {
    root.insert(Outbound());
    if(t.id == opordel && !t.instance.compare(";"))
    {
      root.insert(t);
      t = scan(in);
      return root;
    }
    return root;
  }
  else if(t.id == keyword && !t.instance.compare("start"))
  {
    root.insert(Block());
    return root;
  }
  else if(t.id == keyword && !t.instance.compare("if"))
  {
    root.insert(If());
    if(t.id == opordel && !t.instance.compare(";"))
    {
      root.insert(t);
      t = scan(in);
      return root;
    }
    return root;
  }
  else if(t.id == keyword && !t.instance.compare("while"))
  {
    root.insert(Loop());
    if(t.id == opordel && !t.instance.compare(";"))
    {
      root.insert(t);
      t = scan(in);
      return root;
    }
    return root;
  }
  else if(t.id == keyword && !t.instance.compare("assign"))
  {
    root.insert(Assign());
    if(t.id == opordel && !t.instance.compare(";"))
    {
      root.insert(t);
      t = scan(in);
      return root;
    }
    return root;
  }
  else if(t.id == keyword && !t.instance.compare("jump")) 
  {
    root.insert(Jump());
    if(t.id == opordel && !t.instance.compare(";"))
    {
      root.insert(t);
      t = scan(in);
      return root;
    }
    return root;
  }
  else if(t.id == keyword && !t.instance.compare("label")) 
  {
    root.insert(Label());
    if(t.id == opordel && !t.instance.compare(";"))
    {
      root.insert(t);
      t = scan(in);
      return root;
    }
    return root;
  }

  parseErr("kw_tok : {start, if, loop, assign, or label}");
}

// <inbound> -> kw_tok("listen") id_tok
static node<std::string> Inbound()
{
  node<std::string> root("<inbound>");

  if(t.id == keyword && !t.instance.compare("listen"))
  {
    root.insert(t);
    t = scan(in);

    if(t.id == identifier)
    {
      root.insert(t);
      t = scan(in);
      return root;
    }
    parseErr("id_tok or op_tok");
  }
  parseErr("kw_tok: 'listen'");
  return root;
}

// <outbound> -> kw_tok("talk") <expr>
static node<std::string> Outbound()
{
  node<std::string> root("<outbound>");

  if(t.id == keyword && !t.instance.compare("talk")) 
  {
    root.insert(t);
    t = scan(in);
    root.insert(Expr());
    return root;
  }
  parseErr("kw_tok: 'talk'");
  return root;
}

// <if> -> kw_tok("if") op_tok([) <expr> <RelOp> <expr> op_tok(]) kw_tok("then") <stat>
static node<std::string> If()
{
  node<std::string> root("<if>");

  if(t.id == keyword && !t.instance.compare("if"))
  {
    root.insert(t);
    t = scan(in);

    if(t.id == opordel && !t.instance.compare("["))
    {
      root.insert(t);
      t = scan(in);

      root.insert(Expr());
      RelOp(root);
      root.insert(Expr());

      if(t.id == opordel && !t.instance.compare("]"))
      {
        root.insert(t);
        t = scan(in);
      }

      if(t.id == keyword && !t.instance.compare("then"))
      {
        root.insert(t);
        t = scan(in);
      }
      root.insert(Stat());
      return root;
      parseErr("op_tok: ']'");
    }
    parseErr("op_tok: '['");
  }
  parseErr("kw_tok: 'if'");

  return root;
}

// <loop> -> kw_tok("loop") op_tok([) <expr> <RelOp> <expr> op_tok(]) <stat>
static node<std::string> Loop()
{
  node<std::string> root("<loop>");

  if(t.id == keyword && !t.instance.compare("while"))
  {
    root.insert(t);
    t = scan(in);

    if(t.id == opordel && !t.instance.compare("["))
    {
      root.insert(t);
      t = scan(in);

      root.insert(Expr());
      RelOp(root);
      root.insert(Expr());

      if(t.id == opordel && !t.instance.compare("]"))
      {
        root.insert(t);
        t = scan(in);
      }
      root.insert(Stat());
      return root;
      parseErr("opTK: ']'");
    }
    parseErr("op_tok: '['");
  }
  parseErr("kw_tok: 'while'");
  return root;
}

//<assign> -> kw_tok("assign") id_tok op_tok(=) <expr>
static node<std::string> Assign()
{
  node<std::string> root("<assign>");
  if(t.id == keyword && !t.instance.compare("assign"))
  {
    root.insert(t);
    t = scan(in);
    if(t.id == identifier)
    {
      root.insert(t);
      t = scan(in);


      if(t.id == opordel && !t.instance.compare("="))
      {
        root.insert(t);
        t = scan(in);

        root.insert(Expr());
        return root;
      }
     
      parseErr("op_tok: '='");
    }
    parseErr("id_tok");
    return root;
  }
}

// <RelOp> -> op_tok(>) | op_tok(<) | op_tok(==) | op_tok([) op_tok(==) op_tok(]) | op_tok(%)
static void RelOp(node<std::string>& root)
{
  //node<std::string> root("<RelOp>");

  if(t.id == opordel && !t.instance.compare(">"))
  {
    root.insert(t);
    t = scan(in);
    return;
  }

  else if(t.id == opordel && !t.instance.compare("<"))
  {
    root.insert(t);
    t = scan(in);
    return;
  }
  else if(t.id == opordel && !t.instance.compare("=="))
  {
    root.insert(t);
    t = scan(in);
    return;
  }
  else if(t.id == opordel && !t.instance.compare("["))
  {
    root.insert(t);
    t = scan(in);

    if(t.id == opordel && !t.instance.compare("=="))
    {
      root.insert(t);
      t = scan(in);
    }

    if(t.id == opordel && !t.instance.compare("]"))
    {
      root.insert(t);
      t = scan(in);
      return;
    }
    parseErr("op_tok: ']'");
  }

  else if (t.id == opordel && !t.instance.compare("{"))
  {
    root.insert(t);
    t = scan(in);
    
    if(t.id == opordel && !t.instance.compare("=="))
    {
      root.insert(t);
      t = scan(in);
    }

    if(t.id == opordel && !t.instance.compare("}"))
    {
      root.insert(t);
      t = scan(in);
      return;
    }
    parseErr("op_tok: '}'");
  }

  else if(t.id == opordel && !t.instance.compare("%"))
  {
    root.insert(t);
    t = scan(in);
    return;
  }
  parseErr("op_tok: '=', '<', '>', '%'");
  return;
}

// <label> -> kw_tok(label) idTK
static node<std::string> Label()
{
  node<std::string> root("<label>");

  if(t.id == keyword && !t.instance.compare("label"))
  {
    root.insert(t);
    t = scan(in);

    if(t.id == identifier)
    {
      root.insert(t);
      t = scan(in);

      return root;
    }
    parseErr("id_tok");
  }
  parseErr("kw_tok: 'label'");
  return root;
}

// <jump> -> kw_tok("jump") id_tok
static node<std::string> Jump()
{
  node<std::string> root("<jump>");

  if(t.id == keyword && !t.instance.compare("jump"))
  {
    root.insert(t);

    t = scan(in);

    if(t.id == identifier)
    {
      root.insert(t);
      t = scan(in);
      return root;
    }
    parseErr("id_tok");
  }
  parseErr("kw_tok: 'jump'");
  return root;
}

// error function to show expected token
static void parseErr(std::string expected)
{
  cout << "\n PARSE ERR \n";

  cout << "Line" << t.posString() << "  \"" << t.instance;

  cout << "\" does not match the expected token : " << expected << ".\n";

  exit(-1);
}
