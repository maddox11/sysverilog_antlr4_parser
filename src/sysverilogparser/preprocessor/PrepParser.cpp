
#include "stdlib.h"
#include "cstring"
#include "VerilogPreProcessor.h"
#include "VerilogPreProcessorTokenManager.h"
#include "CharStream.h"
#include "ErrorHandler.h"
#include "PreProcessorErrorHandler.h"
#include "math.h"
#include <regex>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include "Token.h"
#include "BitS.h"
#include "filename.h"
#include "doxygen.h"
#include "config.h"
#include "PrepParser.h"
#include "fileinfo.h"
#include "verilogjjparser.h"

static const std::string            ANSI_RESET  = "\u001B[0m";
static const std::string            ANSI_RED    = "\u001B[31m";
static const std::string            ANSI_GREEN  = "\u001B[32m";
static const std::string            ANSI_YELLOW = "\u001B[33m";
static const std::string            ANSI_CYAN   = "\u001B[36m";
static const std::string            ANSI_WHITE  = "\u001B[37m";

static FileNameLinkedMap fnDict;
 
static ConcurrentMacroMap<DefMacro> globMacroMap;
static ConcurrentMacroMap<QCString> globIncList;
static bool                         s_init = false;

namespace fs                               = std::filesystem;

namespace prepc
{
const std::string reg_word = "[a-zA-Z_$][a-zA-Z_0-9]*|[\"][^\n\r]+[\"]";
const std::string mword    = "[`][a-zA-Z_$][a-zA-Z_0-9]*";
const std::string reg_key  = "[`][\"]";
const std::string regex2   = "[`][`][a-zA-Z_0-9$]*";
const std::string regMacro = "[`][\\w]+[(][^\n\r]*";
const std::string remCom   = "[/][/][^\n\r\\\\]*[\n\r]|[/][*][^*/]*[*][/]";
const std::string remSen   = "[@][@][@]*[^\n\r\t]*[\n]";
const std::regex  mp(mword);
void              writeMacroWarning(const std::string &name, int i);
void              writeFileWarning(const std::string &name, int i);

std::string  prepMacro(const std::string &s)
{
  std::string temp = s;
  size_t  l    = s.find_first_of("=");
  if (l == std::string::npos)
  {
   // std::cerr << "missing = in macro definition:" << s << "\n";
    return s;
  }
  temp[l] = ' ';
  return temp;
}


void PrepParser::_clearAll()
{
}

void PrepParser::replace(std::string &str, const char *suf, const char *ref)
{
  std::regex regg(suf);
  str = std::regex_replace(str, regg, ref);
}


void PrepParser::trim(std::string &result)
{
  QCString s = result.data();
  result     = s.stripWhiteSpace().data();
}

// PrepParser::PrepParser(bool b) : m_psource(b) {}

PrepParser::PrepParser() {};
PrepParser::PrepParser(bool b, SharedState *sh) : m_psource(b), m_shared(sh) {}

PrepParser::PrepParser(const std::string &s, SharedState *sh) : m_file(s), m_shared(sh) {}

PrepParser::PrepParser(const std::string &s) : m_file(s) {}

std::mutex  mup;
std::string PrepParser::getIncludeFile(const std::string &f)
{
  // std::lock_guard<std::mutex> lock(mup);
  // std::string ff     = f;
  // replaceChar(result, "\"", "");
  std::string result = QCString(f).simplifyWhiteSpace().data();
  size_t      i      = result.find_last_of("/");
  if (i != std::string::npos)
    result = result.substr(i + 1);

  QCString qres = result.data();
  bool     am   = false;
  FileDef *fd   = findFileDef(&fnDict, qres, am);
  //Doxygen::includeNameLinkedMap->add()
  if (fd)
    return fd->getDefFileName().data();

 // QCString *q = globIncList.get(result);
 // if (q)
 //   return q->data();

  return "";
}


std::string PrepParser::parseData(const std::string &str)
{
  setBitField(str);
  //  std::cout << str << "\n";
  CharStream                      *sc           = new CharStream(str.c_str(), str.size(), 1, 1);
  VerilogPreProcessorTokenManager *tokenManager = new VerilogPreProcessorTokenManager(sc);
  VerilogPreProcessor             *parser       = new VerilogPreProcessor(tokenManager);
  parser->setOutlineParser(this);
  PreProcessorErrorHandler *parserErrHandler = new PreProcessorErrorHandler("xxxx");
  parser->setErrorHandler(parserErrHandler);
  tokenManager->setLexParser(parser);

  try
  {
    parser->source_text();
  }
  catch (std::exception &e)
  {
    fprintf(stderr, "\nerror..[%s]\n", e.what());
  }
  delete parser;
  delete tokenManager;
  delete sc;

  std::string r = insertDelLines(buf, this);
  return r;
}

int PrepParser::countChar(const std::string &s, char c)
{
  int count = 0;
  for (size_t j = 0; j < s.length(); j++)
  {
    if (s.at(j) == c)
      count++;
  }
  return count;
}

void PrepParser::readFile(const std::string &fi, std::string &buffer)
{
  buffer = fileToString(QCString(fi), true).data();
}

void PrepParser::setBitField(const std::string &s)
{
  int l = countChar(s, '\n');
  bit.setSize(l + l + 1000);
}

#if 0
DefVec PrepParser::getMacros()
{
  assert(m_shared);
  return m_shared->currMap.getMacros();
}
#endif
void PrepParser::printStack()
{
}

void writeOutput(const char *file, const std::string &text)
{
  std::ofstream outfile(file, std::ofstream::out | std::ofstream::binary);

  if (!outfile.is_open())
  {
    std::cerr << ANSI_RED;
    std::cerr << "could not open file:" << file << std::endl;
    std::cerr << ANSI_RESET;
    return;
  }
  outfile.write(text.c_str(), text.size());
  outfile.close();
}

void PrepParser::parseMacro(const Token *t, DefMacro *def)
{
  bool        bb    = false;
  std::string image = t->image;
  replace(image, "`define", "");
  image = eraseComment(image);
  PrepParser prep(m_file, m_shared);
  prep.setBitField(image);
  CharStream                      *sc               = new CharStream(image.data(), image.size(), 1, 1);
  VerilogPreProcessorTokenManager *tokenManager     = new VerilogPreProcessorTokenManager(sc);
  VerilogPreProcessor             *parser           = new VerilogPreProcessor(tokenManager);
  PreProcessorErrorHandler        *parserErrHandler = new PreProcessorErrorHandler("macro");
  parser->setErrorHandler(parserErrHandler);
  parser->setOutlineParser(&prep);
  tokenManager->setLexParser(parser);
  prep.pMM      = true;
  auto df       = std::make_shared<DefMacro>();

  df->beginLine = t->beginLine;
  df->endLine   = t->endLine;
  df->column    = t->beginColumn;
  try
  {
    bb = parser->parse_macro(df.get());
  }
  catch (std::exception &e)
  {
    fprintf(stderr, "\n error [%s]", e.what());
    assert(false);
  }

  delete parser;
  delete tokenManager;
  delete sc;

  if (bb)
  {
    trim(image);
    prep.buf = image.substr(df->mName.length());
  }

replace(prep.buf, "\\\\\n", "\n");
replace(prep.buf,"@@@@","");
//std::cout<<"\n parse Macro:" << prep.buf.data()<<"\n\n";
 
  std::size_t i = prep.buf.length() - 1;
  std::size_t l = prep.buf.find_last_of("\\");
  if (m_shared == NULL)
    assert(m_shared);
  if (l == i && l != std::string::npos)
  {
    df->mDefinition = prep.buf.substr(0, i);
  }
  else
    df->mDefinition = prep.buf;

  if (def == NULL)
  {
    m_shared->currMap.write(df->mName, df);
    globMacroMap.write(df->mName, df);
  }
  else
    def->copy(df.get());

  prep.pMM = false;
}

std::list<std::string> PrepParser::getArguments(std::string &t)
{
  trim(t);
  std::list<std::string> tl, lis;
  size_t                 e  = t.find_first_of('(');
  size_t                 e1 = t.find_last_of(')');

  // no arguments
  if (e == std::string::npos || e1 == std::string::npos)
  {
    replace(t, "`", "");
    lis.emplace_back(t);
    return lis;
  }
  e++;
  size_t      f    = t.find_last_of(')');
  std::string sc   = t.substr(e, f - e);
  std::string name = t.substr(0, e - 1);
  trim(name);
  lis             = getFuncArguments(sc);
  std::string reg = OUTL;

  for (std::string sv : lis)
  {
    trim(sv);
    replace(sv, reg.c_str(), ",");
    replace(sv, "\\\\\n", "\n");
    tl.emplace_back(sv);
  }

  replace(name, "`", "");
  tl.emplace_back(name);

  return tl;
}

std::list<std::string> PrepParser::getFuncArguments(const std::string &s)
{
  int         count = 0;
  std::string sb;
  bool        found = false;

  for (size_t j = 0; j < s.length(); j++)
  {
    if (s.at(j) == '(')
    {
      found = true;
      count++;
    }

    if (s.at(j) == ')')
    {
      count--;
      if (found && count == 0)
      {
        found = false;
      }
    }

    if (found && s.at(j) == ',')
    {
      //  sb[j] = '@';
      sb.append(OUTL);
      continue;
    }
    sb.append(1, s[j]);
  }

  found           = false;
  std::string str = sb;
  sb.clear();
  for (size_t j = 0; j < str.length(); j++)
  {

    if (str.at(j) == '"' && found)
    {
      found = false;
      sb.append(1, str[j]);
      continue;
    }

    if (str.at(j) == '"')
    {
      found = true;
    }

    if (found && str.at(j) == ',')
    {
      //   sb[j] = '@';
      sb.append(OUTL);
      continue;
    }
    sb.append(1, str[j]);
  }

  std::list<std::string> lii;
  bool                   lb = false;
  count                     = 0;
  std::string buf;

  for (size_t j = 0; j < sb.length(); j++)
  {
    char val = sb.at(j);
    if (val == ',' && !lb)
    {
      replace(buf, "``", "");
      lii.emplace_back(buf);
      buf.clear();
      continue;
    }

    buf += val;
    if (val == '{')
    {
      count++;
      lb = true;
    }

    if (val == '}')
    {
      count--;
      if (count == 0)
      {
        lb = false;
      }
    }
  }
  if (buf.length() > 0)
  {
    //     replace(buf, "\n", "");
    lii.emplace_back(buf);
  }

  return lii;
}

std::string PrepParser::insertDelLines(std::string sb, PrepParser *p)
{
  std::string           buff;
  int                   line = 1;

  // std::cout << sb << "\n";
  std::string::iterator it;
  for (it = sb.begin(); it != sb.end(); it++)
  {
    auto c = *it;
    if (c == '\n')
    {
      buff += c;
      if (p->bit.test(line + 1))
      {
        auto v = *(it + 1);
        if (v != '@')
          buff.append(OUTL);
      }
      line++;
    }
    else
      buff += c;
  } // forck
  return buff;
} // insert


bool PrepParser::getParserStateElif(int l)
{
  int len = ifMap.size();
  if (len == 0)
    return false;

  if (len == 1)
    return ifMap.front()->parse;

  for (auto it = ifMap.rbegin(); it != ifMap.rend(); it++)
  {
    DefCont *c = *it;
    if (c->level < l)
      return !c->parse;
    else
    {
      if (c->level == l && c->parse == false)
        return false;
    }
  }
  return true;
}

bool PrepParser::getParserState(int l)
{
  int len = ifMap.size();
  if (len == 0)
    return false;

  if (len == 1)
    return ifMap.front()->parse;

  for (auto it = ifMap.rbegin(); it != ifMap.rend(); it++)
  {
    DefCont *c = *it;
    if (c->level < l)
      return c->parse;
    else
    {
      if (c->level == l && !c->parse)
        return true;
    }
  }
  return false;
}


void PrepParser::adjust(int l)
{
 // if(ifMap.empty())return;
  DefCont *c = ifMap.back();
  if (c == null)
    return;
  while (c->level == l)
  {
    delete c;
    ifMap.pop_back();
    if (ifMap.empty())
      break;
    c = ifMap.back();
  }
}

bool PrepParser::valIfdef(std::string ifc, const char *image, int line, bool flag, int l)
{
  std::string arg = image;
  replace(arg, ifc.data(), "");
  trim(arg);
  DefCont *def = 0;
  bool     el  = (ifc.find("elsif") != std::string::npos);
  if (el)
  {
    bool z = getParserStateElif(l);
    bool c = (m_shared->currMap.get(arg) != NULL);

    if (!c)
    {
      ifMap.emplace_back(def = new DefCont(arg, ifc, line, true, l));
      return true;
    }

    if (z)
    {
      ifMap.emplace_back(def = new DefCont(arg, ifc, line, false, l));
      return false;
    }

    ifMap.emplace_back(def = new DefCont(arg, ifc, line, true, l));
    return true;
  }
  bool state = getParserState(l);
  if (state)
  { // parserON //!
    ifMap.emplace_back(def = new DefCont(arg, ifc, line, true, l));
    return true;
  }
  else if ((m_shared->currMap.get(arg) != NULL) == flag)
  {
    ifMap.emplace_back(def = new DefCont(arg, ifc, line, false, l));
    return false;
  }
  else
  {
    ifMap.emplace_back(def = new DefCont(arg, ifc, line, true, l));
    return true;
  }
}


int PrepParser::countLines(const std::string &tok, bool in, int bl, int el)
{
  bool b = tok.compare("\n") == 0;
  if (b)
  {
    line++;
    z_old++;
    return 1;
  }
  int l     = countChar(tok, '\n') + 1;
  int z_new = 0;

  if (in)
  {
    z_old = z_old - l + 1 + 1;
    z_new = z_old + l - 2;
  }
  else
    z_new = z_old + l - 1;

  line += l;

  int v = z_new + 2;
  int w = z_old + 1;

  int i = bl;
  int j = el;

  if (i == j)
  {
    bit.set((w), parserOn);
    //    std::cout << "bit:" << w << " [ " << parserOn << " ]\n";
  }
  else
  {
    // from ->tocountLines
    for (int i = w; i < v; i++)
    {
      bit.set(i, parserOn);
    }
  }

  z_old = z_new;
  return l;
}

int PrepParser::countLines(const Token *tok, bool in)
{
  return countLines(tok->image, in, tok->beginLine, tok->endLine);
}

void PrepParser::appNewLines(int n)
{
  for (int j = 0; j < n; j++)
    buf.append("\n");
}

void PrepParser::appStringSpecial(const std::string &str)
{
  int num = countChar(str, '\n');
  bit.setSize(num + 1 + bit.getSize());
  z_old += num;
  appString(str);
}
#if 0
void PrepParser::appString0(const std::string , bool ins)
{
  //  cout << "adding token0:" << tok->image << "\n";
  if (tok == null || tok->image.size() == 0)
    return;

  countLines(tok, ins);
  appString(tok->image);
}
#endif
void PrepParser::appString0(const Token *tok, bool ins)
{
  //  cout << "adding token0:" << tok->image << "\n";
  if (tok == null || tok->image.size() == 0)
    return;

  countLines(tok, ins);
  appString(tok->image);
}

void PrepParser::appString(const std::string &str, bool ins)
{
  countLines(str, ins, 0, 0);
  buf.append(str);
}

void PrepParser::appString(const Token *tok)
{
  // cout << "adding token:" << tok->image << "\n";
  int l = countLines(tok, false);
  appNewLines(l);
}

void PrepParser::appString(const std::string str)
{
  buf.append(str);
}

void PrepParser::appStringSkip(const char *c)
{
  sbb.append(c);
}

void PrepParser::appStringSkip(const Token *t)
{
  sbb.append(t->image);
}

// `timescale `celldefine etc
void PrepParser::appSysMacros(const Token *t)
{
  if(parserOn)
   buf.append(OUTL);
  else
   buf.append("<>"); 
  countLines(t, false);
  std::string str = t->image;
  replace(str, "\n", "");
  appString(str);
  appNewLines(1);
}

void PrepParser::appStringDef(const Token *tok, size_t kind)
{
  if (checkIf(kind))
    appString(OUTL);

  appString(OUTL);
  appString0(tok, false);
}

enum states
{
  ELSE,
  ELIF,
  IFDEF,
  IFNDEF,
  ENDIF
};

bool PrepParser::getPrevLevel(int l)
{
  for (auto it = ifMap.rbegin(); it != ifMap.rend(); it++)
  {
    DefCont *c = *it;
    if (c->level < l)
      return c->parse;
  }
  return false;
}

bool PrepParser::checkIf(size_t kind)
{
  bool b = ifMap.empty();
  int  l = ifMap.size();

  if (l == 1) return true;

  bool bp = getPrevLevel(this->level);

  switch (kind)
  {
  case IFDEF:
  case IFNDEF:
  case ELIF:
  case ELSE:
    if (!parserOn || !bp)
      return true;
    break;

  case ENDIF:
    if (!bp) return true;
    if (!parserOn)
      return true;
    break;
  }

  return false;
}


void PrepParser::endif(const Token *tok)
{

  adjust(level);
  level--;

  appStringDef(tok, ENDIF);

  if (ifMap.size() == 0)
  {
    parserOn = false;
    return;
  }
  DefCont *d = ifMap.back();
  parserOn   = d->parse;
}

void PrepParser::ifdef(const Token *tok)
{
  parserOn = valIfdef("`ifdef", tok->image.c_str(), tok->beginLine, true, ++level);
  appStringDef(tok, IFDEF);
}

void PrepParser::ifndef(const Token *tok)
{
  parserOn = valIfdef("`ifndef", tok->image.c_str(), tok->beginLine, false, ++level);
  appStringDef(tok, IFNDEF);
}

void PrepParser::_else(const Token *tok)
{

  bool state = getParserStateElif(level);

  if (level > 0)
    parserOn = !state;

  appStringDef(tok, ELSE);

  //  std::cout << buf << "\n";
  ifMap.emplace_back(new DefCont(" ", "`else", tok->beginLine, parserOn, level));
}

void PrepParser::elsif(const Token *tok)
{
  parserOn = valIfdef("`elsif", tok->image.c_str(), tok->beginLine, true, level);
  appStringDef(tok, ELIF);
}

void PrepParser::define(const Token *tok)
{
  appString0(tok, false);
  // replace(tok->image, "\n", "");
  macro(tok, true);
}

void PrepParser::macro(const Token *tok, bool b)
{

  if (!b)
    appString0(tok, false);

  bool isDef = (tok->image.find("`define") != std::string::npos);
  if (isDef && parserOn && !m_psource)
    return;

  if (isDef)
    parseMacro(tok);
}

void do_something(std::string &v)
{
}


void PrepParser::include(const Token *tok)
{
  if (parserOn)
  {
    appString(OUTL);
    appString0(tok, false);
    return;
  }

  appString0(tok, false);

  std::string inc = tok->image;
  //std::cout << "search include file:" << inc << "\n";
  replace(inc, "`include", "");
  replace(inc, "\n", "");
  replace(inc, "\"", "");
  std::string result = eraseComment(inc);
  std::string v      = getIncludeFile(result);

  // file not found
  if (v.empty())
  {
    prepc::writeFileWarning(result, tok->beginLine);
    return;
  }

  // file already included
  if (m_shared->incPath.find(v) != m_shared->incPath.end())
  {
    return;
  }
  m_shared->incPath.insert(v);

  //std::cout << ANSI_RED;
  //std::cout << "preprocess file:" << v << "\n";
  //std::cout << ANSI_RESET;

  std::string str;
  PrepParser  p(v, m_shared);
  p.readFile(v, str);
  p.parseData(str);
}

void PrepParser::parseComment(Token *tok)
{
  appString0(tok, false);
}

void PrepParser::undef(const Token *tok)
{
  buf.append(OUTL);
  appString0(tok, false);
  std::string str = tok->image;
  replace(str, "`undef", "");
  trim(str);
  //   PrepParser::cmp.erase(str);
}

std::string PrepParser::getMName(const std::string &s)
{
  size_t      i    = s.find('(');
  std::string name = s.substr(0, i);
  replace(name, "`", "");
  return name;
}

std::mutex mmx;
bool       PrepParser::findExpandAs(const std::string &def, bool *del)
{
  std::lock_guard<std::mutex> lock(mmx);
  if (Doxygen::expandAsDefinedSet.empty() || def.empty())
    return false;

  std::string res = def + "=";

  if (Doxygen::expandAsDefinedSet.find(res.data()) != Doxygen::expandAsDefinedSet.end())
  {
    *del = true;
    return true;
  }

  return (Doxygen::expandAsDefinedSet.find(def.c_str()) != Doxygen::expandAsDefinedSet.end());
}

std::mutex lo;

/*
 
 void readFileOrDirectory(const QCString &s,
                        FileNameLinkedMap *fnDict,
                        StringUnorderedSet *exclSet,
                        const StringVector *patList,
                        const StringVector *exclPatList,
                        StringVector *resultList,
                        StringUnorderedSet *resultSet,
                        bool recursive,
                        bool errorIfNotExist=TRUE,
                        StringUnorderedSet *killSet = nullptr,
                        StringUnorderedSet *paths = nullptr
                       );
 
--------------------------------------
 readFileOrDirectory(s,                             // s
                        Doxygen::includeNameLinkedMap, // fnDict
                        nullptr,                       // exclSet
                        &pl,                           // patList
                        &exclPatterns,                 // exclPatList
                        nullptr,                       // resultList
                        nullptr,                       // resultSet
                        false,                         // INCLUDE_PATH isn't recursive
                        TRUE,                          // errorIfNotExist
                        &killSet);                     // killSet
  }

 */



void   PrepParser::setPredef()
{
  //------------------------------------------------------------
  
  std::lock_guard<std::mutex> lock(lo);
  if (m_shared->g_init) return;
  m_shared->g_init               = true;
  const StringVector &predefList = Config_getList(PREDEFINED);
  for (const auto &ds : predefList)
  {
    std::string vl = prepMacro(ds);
    if (vl.empty()) continue;
    replace(vl, "\\\\", "\n");
    // std::cout << "insert predefined: " << vl.data() << "\n";
    prepc::Token tok;
    tok.image = vl;
    parseMacro(&tok);
  }


  const StringVector &patList = Config_getList(FILE_PATTERNS);


  if (s_init) return;
  s_init  = true;

  //---------------------------------------------
  const StringVector &exclPatterns    = Config_getList(EXCLUDE_PATTERNS);
  bool                alwaysRecursive = Config_getBool(RECURSIVE);
  StringUnorderedSet  excludeNameSet;
  //FileNameLinkedMap *fnDict = new FileNameLinkedMap();
  const StringVector &includePathList = Config_getList(INCLUDE_PATH);

    for (const auto &s : includePathList)
    {
      
      readFileOrDirectory(s,                             // s
                        &fnDict, // fnDict
                        nullptr,                       // exclSet
                        &patList,                           // patList
                        &exclPatterns,                 // exclPatList
                        nullptr,                       // resultList
                        nullptr,                       // resultSet
                        TRUE,                         // INCLUDE_PATH isn't recursive
                        TRUE                         // errorIfNotExist
                        );                     // killSet
                  }// for
      
    
   
  //------------------------------------------
  if (!Config_getBool(MACRO_EXPANSION))
    return;
  //const StringVector &searchMacros = Config_getList(EXCLUDE);
 // FileNameLinkedMap *pf = Doxygen::includeNameLinkedMap;
  // auto it =
  for (auto it = fnDict.begin(); it != fnDict.end(); it++)
  {
    const FileName *fnn = it->get();
    std::string     fi  = fnn->fullName().data();
    prepc::PrepParser p(false, m_shared);
    std::string       str;
    p.readFile(fi, str);
   std::cout << ANSI_CYAN << "\npreprocessing file:" << fi << "\n";
    p.parseData(str);
  }
 

} // setPrepdef
//------------------------------------------
auto const           regexbb      = std::regex("[\"]([\\\\][^\r]|[^\r\n\"\\\\])*[\"]");
auto const           regexpr      = std::regex("[`][\"\\\\]|[\"]([\\\\][^\r]|[^\r\n\"\\\\])*[\"]|[a-zA-Z_$][a-zA-Z_0-9]*|([`][`])");
auto const           regexprMacro = std::regex("[`][\\w]+[\\s]*[(]|[`][a-zA-Z_$][a-zA-Z_0-9]*");

std::recursive_mutex recmutex;

std::string          PrepParser::evalMacro(std::string s1, bool ins, int beginLine)
{

  std::unique_lock<std::recursive_mutex> lock(recmutex);

  bool                                   mexp = Config_getBool(MACRO_EXPANSION);
  // no macro expansion !
  if (!mexp)
  {
    std::string val = "||>" + s1 + "<||";
    //std::string val =  s1  ;
    appString(val, false);
    return "";
  }

  std::string            macroName;
  //std::cout << "eval macro:" << s1 << " : " << beginLine << "\n";
  std::string            vt   = s1;
  std::list<std::string> argL = getArguments(vt);
  bool                   del  = false;
  macroName                   = argL.back();
  bool      expAs             = findExpandAs(macroName, &del);

  DefMacro *def               = m_shared->currMap.get(macroName);
  if (def == NULL)
    def = globMacroMap.get(macroName);

  if (def == null)
  {
    // writeMacroWarning(s1, beginLine);
    std::cout << "\nmacro not found:" << macroName << " in file:" << m_file << " at line:" << beginLine << "\n";
     std::string val = "||>" + s1 + "<||";
    appString(val, false);
    return "";
  }

  // macro `define EMPTY
  if (def->mDefinition.empty())
    return "";

  def->setMacroList(argL);

  std::string      ww = updateMacro(def);
  InsertDefinition id(argL, def);
  std::string      res = match(ww, regexpr, &id);
  //  std::cout << "\n\n [ " << res << " ]\n  \n";
  res                  = match(res, regexprMacro, &id);
  //  std::cout << "\nafter eval:\n [ " << res << " ]\n  \n";
  if (ins)
  {
    appString(res, false);
  }

  return res;
}

std::string PrepParser::match(std::string mac, const std::regex &r, InsertDefinition *gn)
{
  auto        words_begin = std::sregex_iterator(mac.begin(), mac.end(), r);
  auto        words_end   = std::sregex_iterator();
  int         x = 0, y = 0, z = 0, k = 0;
  int         len = mac.length();
  std::string sb;
  std::smatch match;
  std::string suff_str = mac;
  for (std::sregex_iterator i = words_begin; i != words_end; ++i)
  {
    std::string name;
    match           = *i;
    std::string str = match.str();

    suff_str        = match.suffix();
    x               = match.position();
    if (x < k)
      continue;
    y       = match.length() + x;
    int len = str.size();

    if (str[0] == '`' && len >= 2)
    {
      if (str[1] == '"')
        name += '"';
      else if (str[1] == '\\')
        name = "\\";
      else if (str.find('(') != std::string::npos)
      {
        y    = getProto(mac, x);
        k    = y;
        name = mac.substr(x, (y - x));
        name = evalMacro(name, false, 0);
      }
      else if (str[1] == '`')
      {
        name = "";
      }
      else
      {
        name = evalMacro(str, false, 1);
      }
    }
    else if (str[0] == '"')
    {
      name = str;
    }
    else
    {
      name = mac.substr(x, match.length());
      name = gn->parseValue(str, m_shared->currMap);
    }

    int         l  = abs(z - x);
    std::string op = mac.substr(z, l);
    sb.append(op);
    sb.append(name);
    z = y;
  } // regex

  if (len > z)
    sb.append(mac.substr(y));

  return sb;
}

std::string PrepParser::insertMacroName(std::string s, PrepParser *p)
{
  assert(p->m_shared);
  std::string name = p->getMName(s);
  DefMacro   *def  = p->m_shared->currMap.get(name);

  if (def == null)
  {
    replace(s, "`", "");
    return s;
  }
  name = p->evalMacro(s, false, 0);
  return name;
}

std::string InsertDefinition::getValue(std::string s)
{
  if (s[0] == '"')
    return s;
  return d->getMacroParam(s);
}

std::string InsertDefinition::parseValue(std::string s, ConcurrentMacroMap<DefMacro> &map)
{
  if (s.at(0) == '`' && s.at(1) == '`')
  {
    s = s.substr(1);
    return getValue(s);
  }

  if (s.at(0) == '`')
  {
    s        = s.substr(1);
    auto itt = map.get(s);
    if (itt != NULL)
    {
      return itt->mDefinition;
    }
    return "";
  }
  return getValue(s);
}

DefCont::DefCont(const std::string &im, const std::string &s, int l, bool b, int lev) : image(im), ifdef(s), line(l), parse(b), level(lev) {}

void DefCont::debug()
{
  std::string        tab = ("\t\t\t\t\t\t\t\t\t\t");
  std::string        tl  = tab.substr(0, level);
  std::string        ok  = parse ? "ON" : "OFF";
  std::ostringstream os;
  os << " " << tl << " " << ifdef << " " << " [" << ANSI_GREEN << " " << image << " " << ANSI_WHITE << "]  at line:" << line << " at level:" << level << " " + ok;
  std::cout << os.str() << std::endl;
}

std::string PrepParser::updateMacro(DefMacro *def)
{
  std::string val = def->mDefinition;
  int         bl  = countChar(val, '\n');
  this->bit.setSize(bit.getSize() + bl);
  replace(val, "\\\\\n", "\n");
  std::string ww = parseDefinition(val);
  ww.append("\n");
  // remove comments
  replace(ww, remCom.c_str(), "");
  trim(ww);
  return ww;
}

std::string PrepParser::parseDefinition(std::string m)
{
  size_t i  = m.find("`ifdef");
  size_t i1 = m.find("`ifndef");

  if (!(i != std::string::npos || i1 != std::string::npos))
    return m;

  replace(m, OUTL.data(), "");

  PrepParser prep(m_file, m_shared);
  prep.setBitField(m);

  CharStream                      *sc           = new CharStream(m.c_str(), m.size(), 1, 1);
  VerilogPreProcessorTokenManager *tokenManager = new VerilogPreProcessorTokenManager(sc);
  VerilogPreProcessor             *parser       = new VerilogPreProcessor(tokenManager);
  parser->setOutlineParser(&prep);
  tokenManager->setLexParser(parser);
  tokenManager->setParser(parser);
  try
  {
    parser->source_text();
  }
  catch (std::exception &e)
  {
    fprintf(stderr, "\n error: [%s]", e.what());
    assert(false);
  }
  delete parser;
  delete tokenManager;
  delete sc;
 
  std::string si = prep.insertDelLines(prep.buf, &prep);
  si.append("\n");
  replace(si, remSen.c_str(), "");
  return si;
}

int PrepParser::getProto(const std::string &m, int x)
{
  //  std::cout << "get_dm\n";
  //  std::cout << m << "\n\n\n";
  int lp = 0;
  int rp = 0;

  for (size_t j = x; j < m.length(); j++)
  {
    char c = m.at(j);
    if (c == '(')
      lp++;

    if (c == ')')
      rp++;

    if (rp == lp && lp > 0)
    {
      return j + 1;
    }
  }

  return m.length() - 1;
}

bool PrepParser::checkInline(Token *tok)
{
  std::string s = tok->image;
  if (s.find("`__FILE__") != std::string::npos)
  {
    appString("\"" + m_file + "\"");
    return true;
  }
  if (s.find("`__LINE__") != std::string::npos)
  {
    auto s = std::to_string(tok->beginLine);
    appString(s);
    return true;
  }
  if (s.find("`endif") != std::string::npos)
  {
    endif(tok);
    return true;
  }
  else if (s.find("`else") != std::string::npos)
  {
    _else(tok);
    return true;
  }

  return false;
}

//--------------------------------------------------------------
bool b_init = false;



void searchDirectories()
    {
        const StringVector &includePathList = Config_getList(INCLUDE_PATH);
        for (const auto &s : includePathList)
        {

            fs::path pt = s;

            for (auto const &dir_entry : std::filesystem::recursive_directory_iterator{pt})
            {
                PrepParser p;
                std::ostringstream os;
                os << dir_entry;
                std::string fi = os.str();
                p.replace(fi, "\"", "");

                if (!dir_entry.is_directory())
                {
                  std::filesystem::path tmp(fi);
                  std::string buf;
                  p.readFile(fi,buf);
                  if(buf.find("`pragma")!=std::string::npos)
                   std::filesystem::remove(tmp);
                }
            }
        }
       }   

void writeMacroWarning(const std::string &name, int i)
{
  std::cout << ANSI_CYAN;
  std::cout << "warning:  macro [" << name << "] is undefined";
  std::cout << "(and assumed null)" << ANSI_RESET;
  std::cout << " at line:" << i << std::endl;
}

void writeFileWarning(const std::string &name, int i)
{
  std::cout << ANSI_YELLOW;
  std::cout << " file: [" << name << " ]";
  std::cout << ANSI_RESET;
  std::cout << " not found at line:" << i << std::endl;
}
}; // namespace prepc

std::string eraseComment(std::string &line)
{
  std::string res, str;
  // only one line comment
  int         v = countLines(line.c_str());
  if (v > 1)
    return line;
  QCString qc(line);
  int      op = qc.findRev("//");

  if (op > 1)
  {
    res = line.substr(op);
    str = line.substr(0, op);
    return str;
  }
  else
    return line;
}


void prepc::PrepParser::writeMacros()
{
  assert(m_shared);
  if (m_shared)
    globMacroMap.writeMacros();
  // m_shared->currMap.writeMacros();
}

void prepc::PrepParser::clearConcMap()
{
  // prepc::currMap.clear();
   //globMacroMap.clear();  
   //globIncList.clear();
   s_init=false;
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

void DefMacro::copy(DefMacro *that)
{
  if (this != that)
  {
    this->beginLine   = that->beginLine;
    this->endLine     = that->endLine;
    this->mName       = that->mName;
    this->mDefinition = that->mDefinition;
    this->column      = that->column;
    this->li          = that->li;
    this->lm          = that->lm;
    this->hm          = that->hm;
  }
}

void DefMacro::setFileName(std::string f) { fi = f; }

void DefMacro::setMacroList(std::list<std::string> &li)
{
  lm.clear();

  for (std::string str : li)
    lm.emplace_back(str);
}

std::string DefMacro::getMacro(int i)
{
  if (i < (int)lm.size() - 1)
    return lm.at(i);

  return "";
}

std::string DefMacro::getSelfDefiniton(std::string &s)
{
  auto it = hm.find(s);
  if (it != hm.end())
    return it->second;
  return "";
}

int DefMacro::findParam(std::string key)
{
  int i = 0;
  for (std::string str : li)
  {
    if (key.compare(str) == 0)
    {
      return i;
    }
    i++;
  }
  return -1;
}

std::string DefMacro::getMacroParam(std::string &p)
{
  std::string s;
  int         i = findParam(p);
  // nothing to be done
  if (i == -1)
    return p;

  std::string s1 = getMacro(i);
  if (s1.length() > 0)
    return s1;

  std::map<std::string, std::string>::iterator it = hm.find(p);
  bool                                         bb = it != hm.end();
  if (bb)
    s = it->second;

  if (s1.empty() && bb)
    return s;

  return "";
}

void DefMacro::debug()
{
  std::cout << "----------------------------------" << std::endl;
  std::cout << mName << " at line: " << beginLine << std::endl;

  std::cout << "map contains:\n";
  for (auto it = hm.begin(); it != hm.end(); ++it)
    std::cout << it->first << " => " << it->second << '\n';

  for (std::string s : li)
  {
    std::cout << "<" << s << ">\n";
  }
  std::cout << "--------------------------------" << std::endl;
  std::cout << mDefinition << std::endl;
  std::cout << "--------------- -----------------" << std::endl;
}
