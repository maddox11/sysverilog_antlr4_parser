
#include <iostream>
#include <list>
#include "antlr4-runtime.h"
#include "SysVerilogLexer.h"
#include "SysVerilogParser.h"
#include "sysverilog.h"
#include "verilogcode.h"
#include "PrepParser.h"
#include "utility.h"
#include "MacroLexer.h"

using SVP = SysVerilogParser;
using TVec = std::vector<antlr4::Token *>;
using TreeVec = std::vector<antlr4::tree::ParseTree *>;
using MLE=MacroLexer;

QCString className;

void writeMacroID(int l, const char *text, VerilogCodeParser *vc)
{
  CodeExpression e;
  e.setCodeParser(vc);
  antlr4::CommonToken t(54);
  e.writeText(nullptr, text, true);
}

CodeExpression::CodeExpression(){};

void CodeExpression::writeText(const antlr4::Token *tt, const char *text, bool isHidden)
{
  antlr4::ANTLRInputStream input(text);
 
  MacroLexer lexer(&input);
  std::string tep(text);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  TVec v = tokens.getTokens();
  int i = v.size();
  bool bf = FALSE;

  for (int j = 0; j < i; j++)
  {
    antlr4::Token *token = v.at(j);
    // size_t u = token->getChannel();
    size_t u = token->getType();

    std::string str = token->getText();

    // new line token '\n'
    if (u == -1)
      continue;

    if ( u== MLE::MacroId)
    {
      m_Code->codifyMacroString(str);
    }
    else if (u == MLE::NEWLINE)
    {
      m_Code->codifyLines(str.data(), 0, false, false, false);
    }
    else if (u == MLE::Block_comment || u == MLE::One_line_comment)
    {
      m_Code->codifyLines(str.data(), 0, false, true, false);
    }
    else if (u == MLE::Integer || u == MLE::Hex_number || u == MLE::Decimal_number || u == MLE::Binary_number || u == MLE::UNLIT)
    {
      m_Code->writeVerilogFont("vhdllogic", str.c_str());
    }
    else if (u == MLE::String_literal)
    {
      m_Code->writeVerilogFont("keyword", str.c_str());
    }
    else if (u == MLE::Simple_identifier)
    {
      // vc->writeVerilogFont("keyword", str.c_str());
      QCString clScope = checkScope(tt);
      m_Code->codifyWord(str.data(), clScope);
    }
   
    else
    {
      switch (u)
      {
      case MLE::RC:
      case MLE::LC:
      case MLE::RB:
      case MLE::LB:
      case MLE::RP:
      case MLE::LP:
        m_Code->writeVerilogFont("stringliteral", str.c_str());
        //  vc->codifyWord(str.data(), className);
        break;
      default:
      m_Code->writeWord(QCString(str.data()));
      }
    }
  }
}

void writeSysFile(int l, const char *text, VerilogCodeParser *vc)
{

  antlr4::ANTLRInputStream input(text);
  SysVerilogLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();
  checkExtension(vc->getFileName(), "v");

  if (Config_getBool(VERILOG_VERSION) && checkExtension(vc->getFileName(), "v"))
    checkVerilog(tokens);

  SysVerilogParser parser(&tokens);
  antlr4::tree::ParseTree *tree = parser.source_code();

  CodeExpression e(&tokens, &parser, vc->getFileName());
  e.setCodeParser(vc);

  antlr4::tree::ParseTreeWalker twalk = antlr4::tree::ParseTreeWalker();
  twalk.walk(&e, tree);
}

std::vector<antlr4::Token *> CodeExpression::getComments(antlr4::Token *tok)
{
  return com->getHiddenTokensToLeft(tok->getTokenIndex(), 2);
}

void CodeExpression::visitTerminal(antlr4::tree::TerminalNode *node)
{

  QCString clazzScope;
  std::string s = node->getText();
  antlr4::Token *tok = node->getSymbol();

  std::string n1 = node->parent->getText();
  TreeVec tv = node->parent->children;
  // printRules(tv);
  size_t u = tok->getType();
  size_t l = tok->getTokenIndex();

  if (l == INVALID_INDEX)
    return;

  // print all hidden tokens(comments/definition/ws  etc before token)

  TVec vec = com->getHiddenTokensToLeft(l, 2);
  // std::cout << vec2string(vec).data();
  printVecToOutput(vec, codeParser());
  if (u == SVP::EOF)
    return;

  printToken(tok);
 // writeText(tok, s.data(), false);
}

QCString CodeExpression::checkScope(const antlr4::Token *t)
{
  QCString res;
  if (t == nullptr)
    return "";

  int id = t->getTokenIndex();

  if (id - 4 < 0)
    return res;

  if (com->get(id - 3)->getType() == SVP::COLONCOLON)
  {
    res = com->get(id - 4)->getText().data();
    res.append("::");
  }
  if (com->get(id - 1)->getType() == SVP::COLONCOLON)
  {
    res.append(com->get(id - 2)->getText().data());
  }
  return res;
}

void CodeExpression::enterEveryRule(antlr4::ParserRuleContext *ctx)
{
  std::string stf = ctx->getText();
  antlr4::Token *tok = ctx->getStart();
  int id = tok->getTokenIndex();
  int index = ctx->getRuleIndex();
  if (tok->getType() == antlr4::Token::EOF)
    id--;

  antlr4::Token *tik = com->get(id + 1);
  size_t u = tok->getType();

  std::string clazz;

  if (index == SVP::RuleModule_nonansi_header)
  {
    SVP::Module_nonansi_headerContext *mc = static_cast<SVP::Module_nonansi_headerContext *>(ctx);
    std::string clazz = mc->ss()->getText();
    m_Code->addClass(clazz);
    return;
  }
  
  if (index == SVP::RuleClass_declaration)
  {
    SVP::Class_declarationContext *mc = static_cast<SVP::Class_declarationContext *>(ctx);
    if(!mc->ss(0)) return;
    std::string clazz = mc->ss(0)->getText().data();
    m_Code->addClass(clazz);
    #if 0
    if (mc->class_type())
    {
      SVP::Class_typeContext *cp = mc->class_type();
      if (cp->psid())
      {
        clazz = cp->psid()->ss()->getText();
        m_Code->addBaseClass(clazz);
      }
    }

    if (mc->interface_class_type().size() > 0)
    {
      std::vector<SVP::Interface_class_typeContext *> ic = mc->interface_class_type();
      for (SVP::Interface_class_typeContext *vp : ic)
      {
        clazz = vp->psid()->ss()->getText();
        m_Code->addBaseClass(clazz);
      }
    }
    #endif 
  }

  if (index == SVP::RuleInterface_declaration)
  {
    SVP::Interface_declarationContext *id = static_cast<SVP::Interface_declarationContext *>(ctx);
    SVP::Interface_nonansi_headerContext* inh=id->interface_nonansi_header();
    std::string clazz=inh->ss()->getText();
    //std::string cl = id->ss(0)->getText().data();
    m_Code->addClass(clazz);
    return;
  }

  if (index == SVP::RulePackage_declaration)
  {
    SVP::Package_declarationContext *mc = static_cast<SVP::Package_declarationContext *>(ctx);
    // std::cout << "enter package name:________" << mc->ss(0)->getText() << " " << tok->getLine() << "\n";
    std::string clazz = mc->ss(0)->getText().data();
    m_Code->addClass(clazz);
    return;
  }

  if (index == SVP::RuleStruct_union)
  {
    clazz = getStructUnionName(ctx->parent);
    m_Code->addClass(clazz);
  }
} // enterEveryRule

void CodeExpression::exitEveryRule(antlr4::ParserRuleContext *ctx)
{
  antlr4::Token *tok = ctx->getStop();
  int index = ctx->getRuleIndex();
  
  switch(index)
  {
    case SVP::RuleModule_declaration:
    case SVP::RuleClass_declaration:
    case SVP::RulePackage_declaration:
    case SVP::RuleInterface_declaration:
               m_Code->removeClass();
    break;
    case SVP::RuleData_type:
         SVP::Data_typeContext *mc = static_cast<SVP::Data_typeContext *>(ctx);
         TreeVec pt=mc->children;
         antlr4::tree::ParseTree *u=pt.at(0);
         bool b=checkRule(u,SVP::RuleStruct_union);
         if (b)
         {
           m_Code->removeClass();     
         } 
    break;

  }
  
  #if 0
  if (index == SVP::RuleModule_declaration)
  {
    SVP::Module_declarationContext *mc = static_cast<SVP::Module_declarationContext *>(ctx);
    m_Code->removeClass();
  }

  if (index == SVP::RuleClass_declaration)
  {
    SVP::Class_declarationContext *mc = static_cast<SVP::Class_declarationContext *>(ctx);
    m_Code->removeClass();
  }

  if (index == SVP::RulePackage_declaration)
  {
    SVP::Package_declarationContext *mc = static_cast<SVP::Package_declarationContext *>(ctx);
    m_Code->removeClass();
  }

  if(index==SVP::RuleInterface_declaration)
  {
    m_Code->removeClass();
  }

  if (index == SVP::RuleData_type)
  {
    SVP::Data_typeContext *mc = static_cast<SVP::Data_typeContext *>(ctx);
    TreeVec pt=mc->children;
    antlr4::tree::ParseTree *u=pt.at(0);
    bool b=checkRule(u,SVP::RuleStruct_union);
    if (b)
     {
       m_Code->removeClass();     
     } 
  }
  #endif
}

void CodeExpression::printVecToOutput(std::vector<antlr4::Token *> &v, VerilogCodeParser *code)
{
  for (antlr4::Token *tok : v)
  {
    printToken(tok);
  }
}

void CodeExpression::enterModule_nonansi_header(SVP::Module_nonansi_headerContext *ctx)
{
  // std::cout<<"module name:"<<ctx->ss()->getText()<<"\n";
}

void CodeExpression::visitErrorNode(antlr4::tree::ErrorNode *node)
{
  antlr4::Token *tok = node->getSymbol();
  std::string s = node->toStringTree();
  size_t l = tok->getTokenIndex();
  if (l == INVALID_INDEX)
    return;
  TVec vec = com->getHiddenTokensToLeft(l, 2);
  printVecToOutput(vec, codeParser());
  m_Code->codifyLines(s.data(), 0, false, true, false);
}
static bool binc=false;
void CodeExpression::printToken(const antlr4::Token *tok)
{
  std::string str = tok->getText();
  size_t u = tok->getType();
  if (u == SVP::NEWLINE)
  {
    m_Code->codifyLines(str.data(), 0, false, false, false);
  }
  else if (u == SVP::Integer || u == SVP::Hex_number || u == SVP::Decimal_number || u == SVP::Binary_number || u == SVP::UNLIT)
  {
    m_Code->writeVerilogFont("vhdllogic", str.c_str());
  }
  else if (u == SVP::String_literal)
  {
    if(binc){
      m_Code->writeIncludeLink(str);
      binc=false;
    }
    else
    m_Code->writeVerilogFont("keyword", str.c_str());
  }
  else if (u == SVP::Simple_identifier)
  {
     QCString  ll;
     QCString clScope = checkScope(tok);
     if(!clScope.isEmpty())
      ll=stripScope(clScope);
    m_Code->codifyWord(str.data(), clScope);
  }
  else if (u == SVP::T_INCLUDEDIR)
  {
    // next token should be string
    binc=true;
    m_Code->codifyLines(str.data(), 0, true, true, false);
  }
  else if(u==SVP::Sys_macro)
  {
    str=str.substr(2);
    writeText(NULL,str.data(),false);
  }
  else if (u == SVP::Out_line_comment)
  {
    int i= QCString(str).contains('@',false);
    replaceChar(str, prepc::OUTL.data(), " ");
    if(i==4)
     writeText(NULL,str.data(),false);
   else
    m_Code->codifyLines(str.data(), 0, true, true, false);
  }
  else if (u == SVP::Block_comment || u == SVP::One_line_comment)
  {
    replaceChar(str, prepc::OUTL.data(), "");
    m_Code->codifyLines(str.data(), 0, false, true, false);
  }
  else if (u == SVP::Macro)
  {
    writeText(tok, str.data(), true);
  }
  else if (u == SVP::Define)
  {
    writeText(tok, str.data(), true);
  }
  else if (u == SVP::MacroId)
  {
    size_t k = str.length() - 6;
    std::string s = str.substr(3, k);
    writeText(tok, s.data(), true);
  }
  else if (u == SVP::Attribute_instance)
  {
    writeText(tok, str.data(), true);
  }
  else
  {
    m_Code->writeWord(QCString(str.data()));
  }
}

#if 0
- -code-keyword-color: #008000;
--code-type-keyword-color: #604020;
--code-flow-keyword-color: #E08000;
--code-comment-color: #800000;
--code-preprocessor-color: #806020;
--code-string-literal-color: #000680ff;
--code-char-literal-color: #008080;
--code-xml-cdata-color: black;
--code-vhdl-digit-color: #FF00FF;
--code-vhdl-char-color: #000000;
--code-vhdl-keyword-color: #700070;
--code-vhdl-logic-color: #ff00d4ff;
#endif

