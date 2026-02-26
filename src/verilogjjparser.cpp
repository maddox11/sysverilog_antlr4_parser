#include <string>
#include <chrono>
#include "qcstring.h"
#include "containers.h"
#include "verilogjjparser.h"
#include "message.h"
#include "config.h"
#include "doxygen.h"
#include "util.h"
#include "language.h"
#include "commentscan.h"
#include "index.h"
#include "definition.h"
#include "searchindex.h"
#include "outputlist.h"
#include "arguments.h"
#include "types.h"

#include "markdown.h"
#include "Expression.h"
#include <regex>
#include "vhdldocgen.h"
#include "qcstring.h"
#include "moduledef.h"
#include "PrepParser.h"
#include "verilogcode.h"
#include <cassert>
#include <mutex>
#include "antlr4-runtime.h"
#include "utility.h"
#include <string_view>
#include "support/Utf8.h"
#include "portable.h"
using namespace prepc;
using namespace antlrcpp;
using SVL = SysVerilogLexer;

struct VerilogDocInfo
{

    VerilogDocInfo() {}
    VerilogDocInfo(QCString &q, int i, bool b) : doc(q), iDocLine(i), brief(b) {}

    ~VerilogDocInfo()
    {
    }

    void addDocInfo(QCString q, int l, bool b)
    {
      auto inf = std::make_shared<VerilogDocInfo>(q, l, b);
      docList.emplace_back(inf);
    }

    void                                         clear() { docList.clear(); }

    QCString                                     doc;
    int                                          iDocLine = 1;
    bool                                         brief;
    bool                                         pending = false;
    std::vector<std::shared_ptr<VerilogDocInfo>> docList;
};

struct VerilogOutlineParser::Private
{
    void                  parseVerilogfile(const char *inputBuffer, bool inLine);
    VerilogOutlineParser *thisParser    = 0;
    SysVerilogParser     *verilogParser = 0;
    VerilogCodeParser    *vCode         = 0;
    CommentScanner        commentScanner;
    QCString              yyFileName;
    int                   yyLineNr = 1;
    int                   iDocLine = -1;
    QCString              inputString;
    Entry                *previous = 0;
    Entry                *oldEntry = 0;
    EntryList             lineEntry;
    VerilogDocInfo        str_doc;
    int                   iCodeLen;
    SharedState           shared;
    int                   code = 0;
};

QCString filterVerilogComment(const QCString &s);

void     printEntries(std::shared_ptr<Entry> ce);

QCString stripStar(std::string q);

void     VerilogOutlineParser::parsePrototype(const QCString &text)
{
}

void copyDoc(std::shared_ptr<Entry> &dst, std::shared_ptr<Entry> &src)
{
  if (!src->brief.isEmpty())
  {
    dst->brief     = src->brief;
    dst->briefLine = src->briefLine;
    dst->briefFile = src->briefFile;
  }
  else
  {
    dst->doc     = src->doc;
    dst->docLine = src->docLine;
    dst->docFile = src->docFile;
  }
}

void VerilogOutlineParser::Private::parseVerilogfile(const char *inputBuffer, bool inLine)
{
  std::size_t numThreads = static_cast<std::size_t>(Config_getInt(NUM_PROC_THREADS));

  // searchDirectories();
  // std::map<std::string, prepc::DefMacro *> mdf;

  std::string s          = inputBuffer;
  std::string ffh        = stripPath(yyFileName).data();
  if (Config_getBool(ENABLE_PREPROCESSING))
  {
    PrepParser pre(ffh, &shared);
    pre.setPredef();
    s = pre.parseData(s);

    #if 0
    if (numThreads == 1)
    {
      prepc::writeOutput("precomp.txt", s);
      pre.writeMacros();
     // searchDirectories();
     // exit(0);
    }
    #endif
  }

  antlr4::ANTLRInputStream  input(s.c_str());
  SysVerilogLexer           lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  tokens.fill();
  #if 0
  if (numThreads == 1)
  {
    std::ofstream f = Portable::openOutputStream("Tokens.txt");
    if (f.is_open())
    {
      TextStream t(&f);
      for (antlr4::Token *tok : tokens.getTokens())
      {
        uint typ = tok->getType();
        if (typ == SVL::WS || typ == SVL::NEWLINE)
          continue;
        t << " " << tok->getText() << " | " << typ << " | " << tok->getLine() << "\n";
        t << "-------------------------------\n";
      }
    }
  }
  #endif
  // convert SystemVerilog keywords in Verilog files in Simple_identifier
  if (Config_getBool(VERILOG_VERSION) && checkExtension(yyFileName, ".v"))
    checkVerilog(tokens);

  SysVerilogParser         parser(&tokens);
  antlr4::tree::ParseTree *tree = parser.source_code();

  Expression              *e    = new Expression(&tokens, &parser, &shared);
  e->setOutlineParser(thisParser);

  // std::cout << "=========  walking source tree:  =============\n";

  antlr4::tree::ParseTreeWalker *twalk = new antlr4::tree::ParseTreeWalker();
  try
  {
    #if 0
    if (numThreads == 1)
    {
      std::string pt = antlr4::tree::Trees::toStringTree(tree, parser.getRuleNames(), TRUE);
      // std::string fii1 = "parsetree.txt";
      prepc::writeOutput("parsetree.txt", pt);
    }
    #endif
    twalk->walk(e, tree);
  }
  catch (std::exception &ex)
  {
    std::cout << ex.what();
  }
  delete twalk;
  delete e;
}

VerilogOutlineParser::VerilogOutlineParser() : p(std::make_unique<Private>())
{
}

VerilogOutlineParser::~VerilogOutlineParser()
{
}

void VerilogOutlineParser::parseInput(const QCString &fileName, const char *fileBuf,
                                      const std::shared_ptr<Entry> &root, ClangTUParser *)
{
  //  verilog file..." << fileName.data() << std::endl;
  SharedState *s  = &p->shared;
  p->thisParser   = this;
  p->inputString  = fileBuf;
  p->yyFileName   = fileName;
  s->fileName     = fileName.data();
  p->yyLineNr     = 1;
  s->current_root = root;
  s->lastCompound = 0;
  s->lastEntity   = 0;
  s->current      = std::make_shared<Entry>();
  initEntry(s->current.get());
  p->parseVerilogfile(fileBuf, false);

  insertPendingComments();
  Doxygen::macroDefinitions.emplace(std::make_pair(s->fileName.c_str(), std::move(s->defList)));

  s->defList.clear();
  s->current.reset();
  // p->yyFileName.resize(0);
}

void VerilogOutlineParser::newEntry()
{
  SharedState *s = &p->shared;
  s->tempEntry   = s->current;
  // std::cout << "add to current_root:[  " << s->current->name << "]  1brief:[ " << s->current->brief << " ] 2line:[ " << s->current->briefLine << " ] [ " << s->current->startLine << " ]\n";

  if (s->current->section.isClass())
  {

    if (s->shvec.empty())
      return;

    if (s->shvec.size() == 1)
    {
      s->current_root->moveToSubEntryAndRefresh(s->current);
    }
    else
    {
      int                    l    = s->shvec.size();
      std::shared_ptr<Entry> temp = s->shvec.at(l - 2);
      // std::cout << temp->name << " adding class : -> " << s->current->name << "\n";
      temp->moveToSubEntryAndRefresh(s->current);
    }

    return;

  } // ins class

  if (s->lastEntity == NULL)
  {
    s->current_root->moveToSubEntryAndRefresh(s->current);
  }
  else
  {
    // std::cout << "adding last entity: " << s->lastEntity->name << " -> " << s->current->name << "\n";
    s->lastEntity->moveToSubEntryAndRefresh(s->current);
  }
  // std::cout<<"lastEntity name:"<<s->lastEntity->name.data()<<"\n";;
  initEntry(s->current.get());
}

void VerilogOutlineParser::initEntry(Entry *e)
{
  e->fileName = p->yyFileName;
  e->lang     = SrcLangExt::Cpp;
  p->commentScanner.initGroupInfo(e);
}

void VerilogOutlineParser::addClass(const char *name, int startLine, int bodyLine, int endBodyLine, EntryType section,
                                    TypeSpecifier ts, Protection prot, const char *type)
{
  SharedState *s       = &p->shared;
  s->current->bodyLine = bodyLine;
  addClass(name, startLine, endBodyLine, section, ts, prot, type);
}

// adding module/class/interface/program/checker
void VerilogOutlineParser::addClass(const char *name, int startLine, int endLine, EntryType section,
                                    TypeSpecifier ts, Protection prot, const char *type)
{
  SharedState *s        = &p->shared;
  std::string  xx       = type;
  s->current->name      = name;
  s->current->startLine = startLine;
  s->current->bodyLine  = startLine;
  s->insertClassEntry(s->current);

  s->current->endBodyLine = endLine;
  s->current->qualifiers.push_back(xx);
  s->current->section    = section;
  // name of inner classes AAA::BBB etc.
  s->current->fileName   = p->yyFileName;
  s->current->spec       = ts;
  s->current->protection = prot;
  s->current->type       = type;
  newEntry();
}

// adding function
#if 0
void VerilogOutlineParser::addVerilogType(const char *n, int startLine, int endLine, EntryType section, TypeSpecifier ts, bool isStatic, Protection prot, const char *qual)
{
  SharedState *s = &p->shared;
  s->current->lang = SrcLangExt_Cpp;
  s->current->startLine = startLine;
  s->current->bodyLine = startLine;
  s->current->endBodyLine = endLine;
  s->current->isStatic = isStatic;
  s->current->section = section;
  s->current->protection = prot;

  s->current->name = n;
  // s->current->args += "( int a, double c) ";
  // s->current->argList.setConstSpecifier(TRUE);
  // s->current->argList.setTrailingReturnType(QCString("double"));
  // s->current->args += " int a ";
  s->current->type = qual;
  // s->current->type.append(" automatic");
  s->current->fileName = p->yyFileName;
  s->current->spec = ts;

  if (qual)
  {
    std::string sq = qual;
    s->current->qualifiers.push_back(sq);
  }

  newEntry();
}
#endif
void VerilogOutlineParser::addMember(const char   *n,
                                     const char   *arg,
                                     const char   *type,
                                     int           startLine,
                                     int           endLine,
                                     EntryType     section,
                                     TypeSpecifier spec,
                                     const char   *exp,
                                     Protection    prot,
                                     const char   *qual,
                                     bool          st)
{
  SharedState *s = &p->shared;
  // if macro xxx is undefined  `xxx(a,b)
  if (n == NULL || isspace(n[0]))
    s->current->name = "dummy";
  else
    s->current->name = n;
  s->current->lang        = SrcLangExt::Cpp;
  s->current->startLine   = startLine;
  s->current->bodyLine    = startLine;
  s->current->endBodyLine = endLine;
  s->current->section     = section;
  s->current->spec        = spec;
  s->current->fileName    = p->yyFileName;
  s->current->type        = type;
  s->current->args        = arg;
  s->current->exception   = exp;
  s->current->isStatic    = st;
  if (exp && s->current->exception != "oop")
  {
    s->current->type.prepend(" ");
    s->current->type.prepend(exp);
    s->current->exception.resize(0);
  }
  //   s->current->bitfields = exp;

  if (qual)
  {
    std::string sq = qual;
    s->current->qualifiers.push_back(sq);
  }

  if (s->isProto)
  {
    s->current->proto            = true;
    s->current->explicitExternal = true;
    s->current->isStatic         = true;
    s->isProto                   = false;
  }

  s->current->protection = prot;
  newEntry();
}

void VerilogOutlineParser::specialHandleCommentBlock(QCString &doc, int line, bool brief, Entry *ent)
{
  int                 position   = 0;
  bool                needs      = FALSE;
  Protection          protection = Protection::Public;
  Markdown            markdown(p->yyFileName, line);
  QCString            processedDoc = Config_getBool(MARKDOWN_SUPPORT) ? markdown.process(doc, line) : doc;
  GuardedSectionStack guards;
  while (p->commentScanner.parseCommentBlock(
    p->thisParser,
    ent,
    processedDoc,  // text
    p->yyFileName, // file
    line,          // line of block start
    true,
    0,
    FALSE,
    protection,
    position,
    needs,
    Config_getBool(MARKDOWN_SUPPORT),
    &guards))
  {
  }
} // special handle

void VerilogOutlineParser::handleCommentBlock(const QCString &doc1, int line, bool brief)
{
  int          position = 0;
  bool         needs    = FALSE;

  SharedState *s        = &p->shared;
  QCString     doc      = doc1;

  if (doc.isEmpty())
    return;

  Protection protection = Protection::Public;
  doc                   = filterVerilogComment(doc1);
  if (checkMultiComment(doc, line))
  {
    return;
  }

  std::shared_ptr<Entry> ent = std::make_shared<Entry>();

  std::shared_ptr<Entry> see = getLastEntry();
  Entry                 *e   = see.get();

  if (e != nullptr && line > e->startLine)
  {
    p->str_doc.addDocInfo(doc1, line, brief);
    return;
  }

  p->oldEntry = s->current.get();

  if (brief)
  {
    ent->briefLine = line;
  }
  else
  {
    ent->docLine = line;
  }

  // std::cout << "handle doc:\n [" << doc.data() << "] {" << line << "}\n";

  Markdown            markdown(p->yyFileName, line);
  QCString            processedDoc = Config_getBool(MARKDOWN_SUPPORT) ? markdown.process(doc, line) : doc;
  GuardedSectionStack guards;

  while (p->commentScanner.parseCommentBlock(
    p->thisParser,
    ent.get(),
    processedDoc,  // text
    p->yyFileName, // file
    line,          // line of block start
    true,
    0,
    FALSE,
    protection,
    position,
    needs,
    Config_getBool(MARKDOWN_SUPPORT),
    &guards))
  {
    if (needs)
      newEntry();
  }

  if (needs)
  {
    s->current_root->moveToSubEntryAndRefresh(ent);
    // copyDoc(s->current,ent);
    // newEntry();
    return;
  }

  if (e == nullptr || e->type == "@")
  {
    p->str_doc.addDocInfo(doc1, line, brief);
    return;
  }

  if (!ent->brief.isEmpty())
  {
    see->brief     = ent->brief;
    see->briefLine = ent->briefLine;
    see->briefFile = ent->briefFile;
  }
  if (!ent->doc.isEmpty())
  {
    see->doc     = ent->doc;
    see->docLine = ent->docLine;
    see->docFile = ent->docFile;
  }
  // s->current.reset();
}

QCString stripStar(std::string q)
{
  if (q.find("*") == std::string::npos)
    return q.data();

  QCString temp;
  uint     len = q.length();
  q.append(" ");
  for (uint j = 0; j < len; j++)
  {
    char c  = q.at(j);
    char c1 = q.at(j + 1);
    if (c == '*' && c1 == ' ')
    {
      temp.append("\\n");
    }
    else
      temp.append(c);
  }

  temp = temp.simplifyWhiteSpace();

  return temp.data();
}

void VerilogOutlineParser::addImport(QCString &fileName, int l, QCString &impName)
{
  const QCString val;
  ModuleManager::instance().addImport(fileName, l, impName, false, val);
}

void VerilogOutlineParser::removeLastEntry(int line)
{
  SharedState                 *s = &p->shared;
 
  if (s->lastEntity && !s->lastEntity->children().empty())
  {
    Entry *e = s->lastEntity->children().back().get();
    if (e->startLine == line)
    {
      Entry *ent=s->lastEntity.get();
      std::cout<<"\n remove entry:"<<ent->name.data()<<"\n\n";
      ent->removeSubEntry(e);
    }
  }
  else if (!s->current_root->children().empty())
  {
    Entry *ee = s->current_root->children().back().get();
    if (ee->startLine == line)
    {
      Entry *ent=s->current_root.get();
      ent->removeSubEntry(ee);
    }
  }
}

const std::shared_ptr<Entry> VerilogOutlineParser::getLastEntity()
{
  // assert(&p->shared!=nullptr_t);
  SharedState *s = &p->shared;
  assert(s);
  assert(s->lastEntity);
  return s->lastEntity;
}

void VerilogOutlineParser::addMacro(const DefMacro *df, const std::string &co)
{
  SharedState *s   = &p->shared;
  FileDef     *fdd = getFileDef();

  Define       def;
  def.name        = df->mName;
  def.definition  = df->mDefinition;
  def.lineNr      = df->beginLine;
  def.columnNr    = df->column;
  def.fileName    = s->fileName;
  def.fileDef     = fdd;

  // std::cout << "[add macro:" << def.name.data() << "]\n"
  //           << def.definition.data() << "\n------------\n";

  int         i   = df->li.size() - 1;
  int         j   = 0;
  std::string arg = "(";
  for (std::string n : df->li)
  {
    arg.append(n);
    if (j < i)
      arg.append(",");
    j++;
  }
  arg.append(")");
  if (arg.length() > 2)
    def.args = arg;
  s->defList.push_back(def);
  s->current->name        = df->mName;
  s->current->args        = arg;
  s->current->startLine   = df->beginLine;
  s->current->bodyLine    = df->beginLine;
  s->current->endBodyLine = df->endLine; // + countLines(def.definition.data()) + 1;
  s->current->startColumn = df->column;
  s->current->mtype       = MethodTypes::Method;
  s->current->section     = EntryType::makeDefine();
  s->current->exception   = df->mDefinition;
  // std::string qual="macro";
  // s->current->qualifiers.push_back(qual);

  if (!co.empty())
  {
    s->current->brief     = co.data();
    s->current->briefLine = df->beginLine;
  }
  newEntry();
}

// prints the verilog parsed types at line xxx
void printEntries(std::shared_ptr<Entry> ce)
{
  for (const auto &rt : ce->children())
  {
    std::cout << "entry name: " << rt->name.data() << " " << rt->startLine << " " << rt->bodyLine << "\n";
    if (!rt->brief.isEmpty() || !rt->doc.isEmpty())
      std::cout << rt->name.data() << ": doc:" << rt->doc.data() << "[ " << rt->brief << "] at line: " << rt->briefLine << "\n";

    printEntries(rt);
  }
}

FileDef *VerilogOutlineParser::getFileDef()
{
  SharedState *s  = &p->shared;
  bool         am = true;
  s->fdd          = findFileDef(Doxygen::inputNameLinkedMap, QCString(s->fileName), am);
  return s->fdd;
}

void VerilogOutlineParser::getEntryAtLine(std::shared_ptr<Entry> ce, int line, bool equ)
{
  for (const auto &rt : ce->children())
  {
    if (rt->bodyLine >= line && !equ)
    {
      p->lineEntry.push_back(rt);
      return;
    }

    if (rt->bodyLine == line && equ)
    {
      p->lineEntry.push_back(rt);
      return;
    }
    getEntryAtLine(rt, line, equ);
  }
}

// docs from typedef enum\struct
void VerilogOutlineParser::insertPendingComments()
{

  VerilogDocInfo *pinf = &p->str_doc;
  SharedState    *s    = &p->shared;
  for (std::shared_ptr<VerilogDocInfo> pi : pinf->docList)
  {
    // bool succ = false;
    getEntryAtLine(s->current_root, pi->iDocLine, false);
    if (!p->lineEntry.empty())
    {
      Entry *e = p->lineEntry.front().get();

      pi->doc  = filterVerilogComment(pi->doc);
      specialHandleCommentBlock(pi->doc, pi->iDocLine, pi->brief, e);
      // std::cout << pi->doc << "\n";

#if 0
      if (pi->brief)
      {
        e->brief     = pi->doc;
        e->briefLine = pi->iDocLine;
      }
      
      {
        e->doc     = pi->doc;
        e->docLine = pi->iDocLine;
      }
#endif
      p->lineEntry.clear();
    }
    // insertCommentAtLine(s->current_root, pi->iDocLine, pi->doc, &succ);
  }
}

bool VerilogOutlineParser::checkMultiComment(QCString &qcs, int line)
{
  SharedState *s = &p->shared;
  getEntryAtLine(s->current_root, line, true);

  if (p->lineEntry.empty())
    return false;

  while (!p->lineEntry.empty())
  {
    std::shared_ptr<Entry> e = p->lineEntry.back();
    e->briefLine             = line;
    e->brief += qcs;

    p->lineEntry.pop_back();
  }
  return true;
}

void VerilogOutlineParser::addComment(std::string &str, size_t line, bool brief)
{
  QCString doc(str.data());
  handleCommentBlock(doc, line, brief);
}

std::shared_ptr<Entry> VerilogOutlineParser::getLastEntry()
{
  std::vector<std::shared_ptr<Entry>> zz;
  SharedState                        *s = &p->shared;

  if (s->current_root == NULL)
    return NULL;

  if (s->lastEntity != nullptr)
  {
    zz = s->lastEntity->children();
    if (zz.empty())
      return s->lastEntity;
  }
  else
    zz = s->current_root->children();

  if (!zz.empty())
  {
    return zz.back();
    //  return m_sharedState->current_root;
  }
  else
    std::cout << "class stack is empty";

  return nullptr;
}

QCString filterVerilogComment(const QCString &s)
{
  if (s.length() < 4) return s;
  QCString result;
  result.reserve(s.length());
  const char *p = s.data() + 3; // skip /*!
  char        c = '\0';
  while (*p == ' ' || *p == '\t') p++;
  while ((c = *p++))
  {
    result += c;
    if (c == '\n')
    {
      // special handling of space followed by * at beginning of line
      while (*p == ' ' || *p == '\t') p++;
      while (*p == '*') p++;
      // special attention in case character at end is /
      if (*p == '/') p++;
    }
  }
  // special attention in case */ at end of last line
  size_t len = result.length();
  if (len >= 2 && result[len - 1] == '/' && result[len - 2] == '*')
  {
    len -= 2;
    while (len > 0 && result[len - 1] == '*') len--;
    while (len > 0 && ((c = result[len - 1]) == ' ' || c == '\t')) len--;
    result.resize(len);
  }
  return stripStar(result.data());
}
