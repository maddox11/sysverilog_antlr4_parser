
#include <vector>

#include "markdown.h"
#include "config.h"
#include "moduledef.h"
#include "defargs.h"
#include "utility.h"

#include "Expression.h"

using namespace antlr4;
using namespace antlr4::tree;
using SVP     = SysVerilogParser;
using Vec     = std::vector<ParseTree *>;
using TVec    = std::vector<Token *>;
using TermVec = std::vector<TerminalNode *>;

struct Sum
{
    void operator()(RuleContext *n)
    {
      sum.append(n->getText().data());
      sum.append(" ");
    }
    QCString sum = "";
};


QCString Expression::getProcessNumber()
{
  QCString qcs;
  char     buf[8];
  qsnprintf(buf, 8, "%d", pCount++);
  qcs.append(&buf[0]);
  return qcs;
}

std::string getSysMacro(std::string &s)
{
  const size_t pos = std::string::npos;
  size_t       l   = s.find("`timescale");
  if (l != pos)
  {
    s = s.substr(l + 10);
    return "timescale";
  }
  l = s.find("`default_nettype");
  if (l != pos)
  {
    s = s.substr(l + 16);
    return "default_nettype";
  }

  return "";
}



bool checkPath(antlr4::tree::ParseTree *t)
{

  bool b = false;
  while (t != NULL)
  {
    if (t->getTreeType() == ParseTreeType::RULE)
    {
     //RuleContext *rc = (RuleContext *)t;
     antlr4::ParserRuleContext* rc=static_cast<antlr4::ParserRuleContext*>(t);

      size_t       bn = rc->getRuleIndex();
      switch(bn)
      {
        case SVP::RuleLoop_generate_construct:
        case SVP::RuleInitial_construct:
        case SVP::RuleAlways_construct:
        case SVP::RuleTask_declaration:
        case SVP::RuleFunction_declaration:
        case SVP::RuleClass_constructor_declaration:
           return true;
        break;   
      }
      #if 0
      if (bn==SVP::RuleLoop_generate_construct || bn==SVP::RuleInitial_construct ||  bn== SysVerilogParser::RuleAlways_construct || bn == SysVerilogParser::RuleFunction_declaration || bn == SVP::RuleTask_declaration)
      {
        // std::cout << "rule .. found:" << bn << "\n";
        return true;
      }
      #endif
    }
    t = t->parent;
  }
  return b;
}

void containsTreeType(ParseTree *pt, size_t rule, std::string &res)
{
  for (ParseTree *t : pt->children)
  {
    if (t->getTreeType() == ParseTreeType::RULE)
    {
      RuleContext *rc = (RuleContext *)t;
      size_t       bn = rc->getRuleIndex();
      // std::cout << "rule index: " << rc->toStringTree() <<bn<< "\n";
      if (bn == rule)
      {
        res.append(rc->getText());
        res.append(" ");
      }
    }
    containsTreeType(t, rule, res);
  } // for
}

// parse hidden tokens like comments //! whitespace,macros etc
std::string Expression::parseHiddenTokens(const antlr4::Token *tok)
{
  std::string buf, se;

  TVec        vec = com->getHiddenTokensToLeft(tok->getTokenIndex(), 2);

  for (Token *t : vec)
  {
    se         = t->getText();
    size_t typ = t->getType();
    replaceChar(se, "\n", "");
    uint lii = t->getLine();
    if (typ == SysVerilogLexer::One_line_comment)
    {
      if (se.find("//!", 0, 3) != std::string::npos)
      {
        outlineParser()->addComment(se, t->getLine(), TRUE);
      }
    }
    else if (typ == SysVerilogLexer::Block_comment)
    {
      if (se.find("/**", 0, 3) != std::string::npos || se.find("/*!", 0, 3) != std::string::npos)
      {

        if (countLines(se.data()) == 1)
          outlineParser()->addComment(se, t->getLine(), TRUE);
        else
          outlineParser()->addComment(se, t->getLine(), FALSE);
      }
    }
    else if (typ == SysVerilogLexer::Macro || typ == SysVerilogLexer::Define)
    {
      std::string res = t->getText();
      if (typ == SysVerilogLexer::Define)
      {
        std::string val = eraseComment(res);
        size_t i=res.find("//!");
        if (i != std::string::npos)
        {
          buf = res.substr(i+3,res.length()-i-3);
        }
      }
      prepc::Token      tt(8, res);
      DefMacro          dm;
      SharedState       sh;
      prepc::PrepParser p(true, &sh);
      p.parseMacro(&tt, &dm);
      dm.beginLine = t->getLine();
      dm.endLine   = dm.beginLine + countLines(dm.mDefinition.c_str());
      outlineParser()->addMacro(&dm, buf);
      if (!buf.empty())
      {
        buf.clear();
      }
    }
    else if (typ == SysVerilogLexer::Attribute_instance)
    {
      //outlineParser()->addMember(id.data(), "", val.data(), l, m, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Protected, "typed");
      QCString attr = "attr";
      attr += getProcessNumber();
      QCString str = "[";
      str += se.substr(2, se.length() - 4).data();
      str += "]";
      outlineParser()->addMember(attr.data(), str.data(), "", lii, lii, EntryType::makeVariable(), TypeSpecifier().setValue(true), "attribute_instance", Protection::Package, "attribute", true);
    }
    else if (typ == SysVerilogLexer::Sys_macro)
    {
      std::string res = getSysMacro(se);
      if (!res.empty())
      {
        outlineParser()->addMember(res.data(), se.data(), "", lii, lii, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Protected, "");
      }
    }
  }

  return buf;
}

void Expression::parseTypeDeclaration(SysVerilogParser::Type_declarationContext *tdc, int l, int m)
{

  QCString               val     = "typedef ";
  int                    ko      = tdc->getStart()->getLine();
  int                    endLine = tdc->getStop()->getLine();
  std::string            id      = tdc->ss(0)->getText();
  int                    line    = tdc->ss(0)->getStart()->getLine();
  std::string            qualifier;
  std::string            un, dat;

  SVP::Data_typeContext *dtc = tdc->data_type();

  if (checkStructUnion(tdc->data_type(), un))
  {
    id = tdc->ss(0)->getText();

    if (tdc->data_type()->T_PACKED())
      qualifier.append("packed ");
    if (tdc->data_type()->signing())
      qualifier.append(tdc->data_type()->signing()->getText());

    TypeSpecifier                    tsp;
    std::list<antlr4::RuleContext *> li;
    // findRule(tdc->parent, SVP::RuleStruct_union, &st);
    getRules(tdc->parent, li, SVP::RuleStruct_union);

    if (li.size() > 0)
    {
      un = li.back()->getText();
      if (un.compare("struct") == 0)
        tsp.setStruct(true);
      else
        tsp.setUnion(true);
    }
   // outlineParser()->addClass(id.c_str(), l, m, endLine, EntryType::makeClass(), tsp, Protection::Private, qualifier.data());
    return;
  }

  if (dtc)
  {
    std::string args;

    parseDataType(tdc->data_type(), args);
    val.append(args);

    if (tdc->data_type()->T_ENUM())
    {
      parseEnumberation(tdc->data_type(), outlineParser(), m_sharedState, id, m);
    }
    else
      outlineParser()->addMember(id.data(), "", val.data(), l, m, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Protected, "typed");

    return;
  }

  if (tdc->DOT())
  {
    QCString v  = tdc->constant_bit_select()->getText().data();
    QCString n0 = tdc->ss(0)->getText().data();
    QCString n1 = tdc->ss(1)->getText().data();
    QCString n2 = tdc->ss(2)->getText().data();
    val.append(n0);
    val.append(v);
    val.append(".");
    val.append(n1);
    outlineParser()->addMember(n2.data(), "", val.data(), l, m, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Public, 0);
    return;
  }

  if (tdc->T_CLASS())
    val.append("class");
  else if (tdc->T_UNION())
    val.append("union");
  else if (tdc->T_ENUM())
    val.append("enum");
  else if (tdc->T_STRUCT())
    val.append("struct");
  else if (tdc->T_INTERFACE())
    val.append("interface");

  if (dtc && dtc->T_ENUM())
    outlineParser()->addMember(id.data(), "", "enum ", l, l, EntryType::makeClass(), TypeSpecifier().setEnum(true), "", Protection::Public, 0);
  else
    outlineParser()->addMember(id.data(), "", val.data(), l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Public, 0);

  if (dtc && dtc->T_ENUM())
  {
    std::vector<SVP::Enum_name_declarationContext *> vs = dtc->enum_name_declaration();

    for (SVP::Enum_name_declarationContext *sc : vs)
    {
      int         line = sc->getStart()->getLine();
      std::string s    = sc->getText();
      //  std::cout << "found enum :" << s << " at line:" << line << "\n";
      std::string id   = sc->ss()->getText();
      outlineParser()->addMember(id.data(), s.data(), "@", l, l, EntryType::makeVariable(), TypeSpecifier().setEnum(true), "", Protection::Public, 0);
    }
  }
}

void parseListofVariablePortIdentifiers(int l, bool ss, const char *r, const char *t, SysVerilogParser::List_of_variable_port_identifiersContext *liof, const char *dir)
{
}

static void parse_tf_port_declaration(std::vector<SysVerilogParser::Tf_item_declarationContext *> &tpi, struct SharedState *sh)
{
  sh->current->argList.clear();
  sh->current->args.resize(0);
  QCString qcs = "(";

  for (SysVerilogParser::Tf_item_declarationContext *s : tpi)
  {
    struct Argument                               ar;
    SysVerilogParser::Tf_port_declarationContext *tp = s->tf_port_declaration();
    if (tp == nullptr)
      continue;
    ar.attrib                                                    = tp->tf_port_direction()->getText();
    ar.type                                                      = tp->data_type_or_implicit()->getText();
    SysVerilogParser::List_of_tf_variable_identifiersContext *lp = tp->list_of_tf_variable_identifiers();

    int                                                       i  = 0;
    for (SysVerilogParser::SsContext *sc : lp->ss())
    {
      ar.name = sc->getText();
      if (lp->expression(i))
        ar.defval = lp->expression(i)->getText();
      i++;
      sh->current->argList.push_back(ar);
    }
  }
}

std::vector<antlr4::Token *> Expression::getHiddenTokens(antlr4::Token *tok, int c)
{
  return com->getHiddenTokensToLeft(tok->getTokenIndex(), c);
}

//-------------------------------------------------------------------------------------------------
void Expression::enterParameter_port_declaration(SysVerilogParser::Parameter_port_declarationContext *ctx)
{
  std::string type;
  //  std::cout << ctx->getText();

  bool        y  = ctx->list_of_param_assignments() != NULL;
  bool        y1 = ctx->data_type() != NULL;
  // bool y2 = ctx->parameter_declaration() != NULL;
  bool        y3 = ctx->list_of_type_assignments() != NULL;
  // bool y4 = ctx->local_parameter_declaration() != NULL;

  /*
    if (y4)
    {
      std::string func, n;
      func = "localparam ";
      SVP::Local_parameter_declarationContext *ld = ctx->local_parameter_declaration();
      if (ld->list_of_param_assignments())
      {
        parseDataTypeImpl(ld->data_type_or_implicit(), n);
        parseListOfParamAssign(ld->list_of_param_assignments(), outlineParser(), func, n);
      }
      if (ld->list_of_type_assignments())
      {
        parseListOfTypeAssign(ld->list_of_type_assignments(), outlineParser(), func, n);
        func.append("type ");
      }
    }
  */
  if (y3)
  {
    std::string func, n;
    func = "type ";
    parseListOfTypeAssign(ctx->list_of_type_assignments(), outlineParser(), func, n,com);
  }

  if (ctx->list_of_param_assignments() && ctx->data_type())
  {
    std::string func, n;
    bool        b = parseDataType(ctx->data_type(), n);
    if (!b)
      func = n;
    parseListOfParamAssign(ctx->list_of_param_assignments(), outlineParser(), func, n,com);
  }
}

void Expression::enterLocal_parameter_declaration(SysVerilogParser::Local_parameter_declarationContext *ctx)
{
  std::string func, n;
  func                                        = "localparam ";
  SVP::Local_parameter_declarationContext *ld = ctx;
  if (ld->list_of_param_assignments())
  {
    parseDataTypeImpl(ld->data_type_or_implicit(), n);
    parseListOfParamAssign(ld->list_of_param_assignments(), outlineParser(), func, n,com);
  }
  if (ld->list_of_type_assignments())
  {
    parseListOfTypeAssign(ld->list_of_type_assignments(), outlineParser(), func, n,com);
    func.append("type ");
  }
}

void Expression::enterParameter_declaration(SysVerilogParser::Parameter_declarationContext *ctx)
{
  std::string                        n;
  bool                               b;
  SVP::Parameter_declarationContext *dec = ctx;

  std::string                        func;
  func = "parameter ";
  if (ctx->T_TYPE())
    func.append("type ");

  b = parseDataTypeImpl(dec->data_type_or_implicit(), n);

  if (!b)
    func.append(n);
  if (dec->list_of_param_assignments())
    parseListOfParamAssign(dec->list_of_param_assignments(), outlineParser(), func, n ,com);
  if (dec->list_of_type_assignments())
    parseListOfTypeAssign(dec->list_of_type_assignments(), outlineParser(), func, n,com);
}

void Expression::enterPort_declaration(SVP::Port_declarationContext *ctx)
{
  std::string                             s, r, t, dir;
  Token                                  *tok = ctx->getStart();

  int                                     l   = tok->getLine();
  SVP::Interface_port_declarationContext *inf = ctx->interface_port_declaration();
  SVP::Output_declarationContext         *out = ctx->output_declaration();
  SVP::Inout_declarationContext          *io  = ctx->inout_declaration();
  SVP::Input_declarationContext          *in  = ctx->input_declaration();
  SVP::Ref_declarationContext            *ref = ctx->ref_declaration();

  bool                                    ss  = false;

  if (inf)
  {
    QCString name;

    name = inf->ss(0)->getText();

    if (inf->DOT())
    {
      name.append(".");
      name.append(inf->ss(1)->getText());
    }

    SVP::List_of_interface_identifiersContext *ii  = inf->list_of_interface_identifiers();
    int                                        len = ii->ss().size();

    for (int j = 0; j < len; j++)
    {
      QCString arg, dim;
      arg = ii->ss(j)->getText();

      for (SVP::Unpacked_dimensionContext *up : ii->unpacked_dimension())
        dim.append(up->getText());

      dim = dim.prepend(name);
      outlineParser()->addMember(arg.data(), dim.data(), "interface port", l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Protected, "Inst");
    }
    return;
  }

  if (in || out || io || ref)
  {
    SVP::List_of_port_identifiersContext          *liof  = nullptr;
    SVP::List_of_variable_port_identifiersContext *liofv = nullptr;
    SVP::Variable_port_typeContext                *vp    = nullptr;
    SVP::List_of_variable_identifiersContext      *lcx   = nullptr;
    SVP::Net_port_typeContext                     *np    = nullptr;

    if (in)
    {
      np   = in->net_port_type();
      dir  = "input ";
      liof = in->list_of_port_identifiers();
      vp   = in->variable_port_type();
      lcx  = in->list_of_variable_identifiers();
    }
    else if (out)
    {
      np    = out->net_port_type();
      dir   = "output ";
      liof  = out->list_of_port_identifiers();
      vp    = out->variable_port_type();
      liofv = out->list_of_variable_port_identifiers();
    }
    else if (io)
    {
      np   = io->net_port_type();
      dir  = "inout ";
      liof = io->list_of_port_identifiers();
    }

    if (liof)
    {
      if (np->net_type())
        r = np->net_type()->getText();

      if (np->data_type_or_implicit())
      {
        t.append(np->data_type_or_implicit()->getText());
        SVP::Data_typeContext *dc = np->data_type_or_implicit()->data_type();
        std::string            vall;
        if (dc)
          ss = parseDataType(dc, vall);
      }

      if (!ss)
      {
        r.append(" ");
        r.append(t);
        t = r;
      }

      parseListofPortIdentifiers(tok->getLine(), ss, r.c_str(), t.c_str(), liof, dir.c_str(), outlineParser());
    }

    if (vp)
    {
      SVP::Var_data_typeContext *vda = vp->var_data_type();
      SVP::Data_typeContext     *vdd = vda->data_type();
      if (vdd == nullptr)
      {
        vdd = vda->data_type_or_implicit()->data_type();
      }
      if (vdd->ss().size() > 0 || vdd->class_type() != nullptr)
        ss = true;

      t.append(vdd->getText());
    }

    parseListofVariableIdentifiers(tok->getLine(), ss, r.c_str(), t.c_str(), lcx, dir.c_str(), outlineParser());
  }
}

void Expression::enterAnsi_port_declaration(SysVerilogParser::Ansi_port_declarationContext *ctx)
{
  QCString s, r, dim, dir, name, arg;
  Token   *tok = ctx->getStart();

  int      l   = tok->getLine();
  if (ctx->ss())
  {
    name = ctx->ss()->getText();
  }
  else
    name = "ZZZ";

  name = ctx->ss() ? ctx->ss()->getText() : "ZZZ";

  if (ctx->constant_expression())
  {
    arg = "=";
    arg.append(ctx->constant_expression()->getText().data());
  }

  if (ctx->expression())
  {
    arg = "(";
    arg.append(ctx->expression()->getText().data());
    arg.append(")");
  }

  std::vector<SVP::Variable_dimensionContext *> vv0 = ctx->variable_dimension();

  if (!ctx->variable_dimension().empty())
    dim = std::for_each(vv0.cbegin(), vv0.cend(), Sum()).sum;

  std::vector<SVP::Unpacked_dimensionContext *> vv1 = ctx->unpacked_dimension();

  if (!ctx->unpacked_dimension().empty())
    dim = std::for_each(vv1.cbegin(), vv1.cend(), Sum()).sum;

  if (ctx->interface_port_header())
  {
    parseInterfaceHeader(ctx->interface_port_header(), r);
    outlineParser()->addMember(name.data(), arg.data(), r.data(), l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Protected, 0);
    return;
  }

  if (ctx->net_port_header())
  {
    // std::cout << "net_port: " << ctx->net_port_header()->getText() << "\n";
    bool     nn = false;
    QCString type1;
    parseNet_Port_Header(ctx->net_port_header(), dir, r, type1, nn);

    dir.append(" ");
    dir.append(type1);

    if (!nn)
    {
      dir.append(" ");
      dir.append(r);
    }

    if (nn)
    {
      name = r;
    }

    outlineParser()->addMember(name.data(), arg.data(), dir.data(), l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Private, 0);
    return;
  }

  if (ctx->variable_port_header())
  {
    parseVariablePortHeader(ctx->variable_port_header(), dir, r);
    dir.append(" ");
    dir.append(r);
    arg = arg.prepend(dim);
    outlineParser()->addMember(name.data(), arg.data(), dir.data(), l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Public, 0);
    return;
  }

  if (ctx->LP() && ctx->RP())
  {
    if (ctx->port_direction())
      dir.append(ctx->port_direction()->getText().data());

    outlineParser()->addMember(name.data(), arg.data(), dir.data(), l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Public, 0);
    return;
  }

  arg = arg.prepend(dim);
  outlineParser()->addMember(name.data(), arg.data(), dir.data(), l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Private, 0);
}

void Expression::enterRef_declaration(SysVerilogParser::Ref_declarationContext *ctx)
{
  Token                             *tok = ctx->getStart();
  SVP::Variable_port_typeContext    *vp  = ctx->variable_port_type();
  SVP::Data_typeContext             *dc  = vp->var_data_type()->data_type();
  SVP::Data_type_or_implicitContext *dic = vp->var_data_type()->data_type_or_implicit();

  std::string                        res;
  bool                               b = false;
  if (dic)
  {
    res.append("var ");
    b = parseDataTypeImpl(dic, res);
  }

  if (dc)
    b = parseDataType(dc, res);

  parseListofVariableIdentifiers(tok->getLine(), b, 0, res.c_str(), ctx->list_of_variable_identifiers(), "| [ ref ]", outlineParser());
}

void Expression::enterPackage_or_generate_item_declaration(SysVerilogParser::Package_or_generate_item_declarationContext *ctx)
{
}

void Expression::enterData_declaration(SVP::Data_declarationContext *ctx)
{

  Token      *s = ctx->getStart();
  Token      *t = ctx->getStop();
  std::string un, dat;
  int         line    = s->getLine();
  int         endLine = t->getLine();
  bool        b       = checkPath(ctx->parent);
  if (b)
    return;

  SVP::Data_typeContext                         *dtc    = NULL;
  SVP::Net_type_declarationContext              *ndc    = ctx->net_type_declaration();
  SVP::Data_type_or_implicitContext             *doc    = ctx->data_type_or_implicit();
  SVP::Net_type_declarationContext              *ntc    = ctx->net_type_declaration();
  SVP::Package_import_declarationContext        *pic    = ctx->package_import_declaration();
  SVP::Type_declarationContext                  *tdc    = ctx->type_declaration();
  SVP::List_of_variable_decl_assignmentsContext *ldtest = ctx->list_of_variable_decl_assignments();

  if (doc)
    dtc = doc->data_type();
  else if (tdc)
    dtc = tdc->data_type();

  if (ndc)
  {
    std::string type, name;

    if (ndc->T_WITH())
    {
      // type = "with ";
      type.append(ndc->ss(1)->getText());
    }

    if (ndc->class_scope())
      type.append(ndc->class_scope()->getText());

    if (ndc->data_type())
      dat = ndc->data_type()->getText();

    std::vector<SVP::SsContext *> vec = ndc->ss();

    if (!vec.empty())
      name.append(ndc->ss(0)->getText());

    outlineParser()->addMember(name.data(), dat.data(), "nettype", line, line, EntryType::makeVariable(), TypeSpecifier().setValue(true), type.data(), Protection::Private, 0);

    return;
  }

  if (pic)
  {
    std::vector<SVP::Package_import_itemContext *> pi = pic->package_import_item();
    for (SVP::Package_import_itemContext *vp : pi)
    {
      std::vector<SVP::SsContext *> n = vp->ss();
      int                           l = n.size();
      if (l == 0)
        continue;
      QCString id = n.at(0)->getText();

      if (l == 2)
      {
        id.append(".");
        id.append(n.at(1)->getText().data());
      }

      outlineParser()->addMember(id.data(), "", "import", line, line, EntryType::makeFunction(), TypeSpecifier().setValue(true), "", Protection::Private, 0, true);
    }
    return;
  }

  if (tdc)
  {
    parseTypeDeclaration(tdc, line, line /*,t->getLine()*/);
    return;
  }

  // checking struct or union
  if(checkStructUnion(dtc, un))
  return;
  #if 0
  if (doc && checkStructUnion(dtc, un))
  {
    TypeSpecifier tsp;

    if (un.compare("struct") == 0)
      tsp.setStruct(true);
    else
      tsp.setUnion(true);

    std::string args, qualifier, id;

    if (dtc->T_PACKED())
      qualifier.append("packed ");
    if (dtc->signing())
      qualifier.append(dtc->signing()->getText());

    bool                                                b   = parseDataTypeImpl(doc, id);

    SVP::List_of_variable_decl_assignmentsContext      *ld  = ctx->list_of_variable_decl_assignments();
    std::vector<SVP::Variable_decl_assignmentContext *> lop = ld->variable_decl_assignment();
    SVP::Variable_decl_assignmentContext               *c   = lop.at(0);
    int                                                 ll  = 0;
    parseVariableDeclAssignment(b, id, args, c, ll);
    if (ll == 0)
      ll = line;

    outlineParser()->addClass(id.c_str(), line, line, endLine, EntryType::makeClass(), tsp, Protection::Public, qualifier.data());
    return;
  }
  #endif

  if (doc)
  {
    QCString    val;

    std::string id, args, res;
    if (doc->data_type() && dtc->T_ENUM())
    {
      SVP::List_of_variable_decl_assignmentsContext      *v1   = ctx->list_of_variable_decl_assignments();
      std::vector<SVP::Variable_decl_assignmentContext *> ju   = v1->variable_decl_assignment();
      int                                                 line = dtc->getStart()->getLine();

      int                                                 i    = 1;
      for (SVP::Variable_decl_assignmentContext *c : ju)
      {
        std::string n = c->getText();
        //  std::cout << "enumdecl " << n.data() << "\n";
        if (i == 1)
          parseEnumberation(dtc, outlineParser(), m_sharedState, n, line);
        else
          outlineParser()->addMember(n.data(), "", "@", line, line, EntryType::makeEnum(), TypeSpecifier().setValue(true), "", Protection::Protected, 0);
        i++;
      }
      return;
    }

    std::string re;
    containsTreeType(ctx->parent, SVP::RuleProperty_qualifier, re);

    if (!re.empty())
    {
      val.append(re);
      val.append(" ");
    }

    if (ctx->T_VAR())
      val.append(" var ");

    if (ctx->lifetime())
      val.append(ctx->lifetime()->getText());

    bool        b  = parseDataTypeImpl(doc, res);
    std::string qq = ctx->data_type_or_implicit()->getText();
    val.append(res.data());

    SVP::List_of_variable_decl_assignmentsContext *ld = ctx->list_of_variable_decl_assignments();
    if (ld)
    {
      qq = ld->getText();
    }

    std::vector<SVP::Variable_decl_assignmentContext *> lop = ld->variable_decl_assignment();

    for (SVP::Variable_decl_assignmentContext *c : lop)
    {
      b      = false;
      int ll = 0;
      parseVariableDeclAssignment(b, id, args, c, ll);
      outlineParser()->addMember(id.data(), args.data(), val.data(), line, line, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Public, "data");
    }
    val.resize(0);

  } //
}

void Expression::enterData_type(SysVerilogParser::Data_typeContext *ctx)
{
}

void Expression::exitStruct_union(SysVerilogParser::Struct_unionContext * ctx)
{

}
void Expression::exitData_declaration(SysVerilogParser::Data_declarationContext * ctx)
{
 
 // std::cout<<"exit data_declaration at line: "<<i<<"\n";
 
}

void Expression::exitStruct_union_member(SysVerilogParser::Struct_union_memberContext * ctx)
{

}



 void Expression::exitData_type_or_void(SysVerilogParser::Data_type_or_voidContext * ctx)
 {
 
 }


void Expression::exitData_type(SysVerilogParser::Data_typeContext *ctx)
{
   std::vector<ParseTree *> pt=ctx->children;
   size_t p=ctx->getStart()->getLine();
   //if(pt.empty()) return;
   ParseTree *u=pt.at(0);
   bool b=checkRule(u,SVP::RuleStruct_union);
   // leaving struct/union block
   if(b){
    m_sharedState->delClassEntry();
   // std::cout<<"exit data type at line:"<<p<<"\n";
   }
   
}


// --------------- PROGRAM --------------------

void Expression::enterProgram_declaration(SysVerilogParser::Program_declarationContext *ctx)
{
  Token                              *s   = ctx->getStart();
  Token                              *t   = ctx->getStop();

  std::string                         lkl = ctx->getText();
  SVP::Program_nonansi_headerContext *pn  = ctx->program_nonansi_header();
  std::string                         cn  = pn->ss()->getText();

  outlineParser()->addClass(cn.c_str(), s->getLine(), t->getLine(), EntryType::makeClass(), TypeSpecifier().setProgram(true), Protection::Public, "program");
}
void Expression::exitProgram_declaration(SysVerilogParser::Program_declarationContext *ctx)
{
  m_sharedState->delClassEntry();
}

//  ------ PACKAGE -------------------

void Expression::enterPackage_declaration(SysVerilogParser::Package_declarationContext *ctx)
{
  Token                        *s = ctx->getStart();
  Token                        *t = ctx->getStop();

  std::string                   life;
  std::string                   cn = ctx->ss(0)->getText();
  std::vector<SVP::SsContext *> ve = ctx->ss();
  if (ctx->lifetime())
    life.append(ctx->lifetime()->getText());
  outlineParser()->addClass(cn.c_str(), s->getLine(), t->getLine(), EntryType::makeClass(), TypeSpecifier().setPackage(true), Protection::Public, "package");
}
void Expression::exitPackage_declaration(SysVerilogParser::Package_declarationContext *ctx)
{
  m_sharedState->delClassEntry();
}

// ---------- INTERFACE CLASS -------------------

void Expression::enterInterface_class_declaration(SysVerilogParser::Interface_class_declarationContext *ctx)
{

  Token      *s  = ctx->getStart();
  Token      *t  = ctx->getStop();
  size_t      g  = s->getLine();

  std::string cn = ctx->ss(0)->getText();
  outlineParser()->addClass(cn.c_str(), s->getLine(), t->getLine(), EntryType::makeClass(), TypeSpecifier().setValue(true), Protection::Public, "interface");

  for (SVP::Interface_class_itemContext *ici : ctx->interface_class_item())
  {
    if (ici->type_declaration())
    {
      size_t u = ici->getStart()->getLine();
      size_t w = ici->getStop()->getLine();
      parseTypeDeclaration(ici->type_declaration(), u, w);
    }
  }
}

void Expression::exitInterface_class_declaration(SysVerilogParser::Interface_class_declarationContext *ctx)
{
  m_sharedState->delClassEntry();
}

void Expression::enterInterface_declaration(SysVerilogParser::Interface_declarationContext *ctx)
{
  Token *s = ctx->getStart();
  Token *t = ctx->getStop();

  if (ctx->interface_class_declaration())
  {

    // SVP::Interface_class_declarationContext *icd = ctx->interface_class_declaration();
    // std::string cn = icd->ss(0)->getText();
    // outlineParser()->addClass(cn.c_str(), s->getLine(), t->getLine(), EntryType::makeClass(), TypeSpecifier().setInterface(true), Protection::Public, "interface class");
    return;
  }

  SVP::Interface_nonansi_headerContext *in = ctx->interface_nonansi_header();

  std::string                           cn = in->ss()->getText();

  outlineParser()->addClass(cn.c_str(), s->getLine(), t->getLine(), EntryType::makeClass(), TypeSpecifier().setValue(true), Protection::Public, "interface");
}

void Expression::exitInterface_declaration(SysVerilogParser::Interface_declarationContext *ctx)
{
  m_sharedState->delClassEntry();
}

// ------------- CHECKER --------------------

void Expression::enterChecker_declaration(SysVerilogParser::Checker_declarationContext *ctx)
{
  Token                        *s  = ctx->getStart();
  Token                        *t  = ctx->getStop();
  std::string                   cn = ctx->ss(0)->getText();
  std::vector<SVP::SsContext *> ve = ctx->ss();

  outlineParser()->addClass(cn.c_str(), s->getLine(), t->getLine(), EntryType::makeClass(), TypeSpecifier().setChecker(true), Protection::Public, "checker");
}
void Expression::exitChecker_declaration(SysVerilogParser::Checker_declarationContext *ctx)
{
  m_sharedState->delClassEntry();
}

//------------ CLASS -------------------------

void Expression::enterClass_property(SysVerilogParser::Class_propertyContext *ctx)
{
  std::string name, arg, func, n;
  QCString    dim;

  if (ctx->data_type())
  {
    func                                                = "const ";
    std::vector<SVP::Class_item_qualifierContext *> vv0 = ctx->class_item_qualifier();
    if (!vv0.empty())
      dim = std::for_each(vv0.cbegin(), vv0.cend(), Sum()).sum;

    func.append(dim.data());

    bool b = parseDataType(ctx->data_type(), n);

    if (ctx->ss())
      name = ctx->ss()->getText();
    else
      name = n;

    if (b)
      func.append(n);

    if (ctx->constant_expression())
    {
      arg = "= ";
      arg += ctx->constant_expression()->getText();
    }
    // std::cout << "record: " << datType.arg.data() << "\n";
    outlineParser()->addMember(name.data(), arg.data(), func.data(), ctx->getStart()->getLine(), ctx->getStart()->getLine(), EntryType::makeVariable(), TypeSpecifier().setDefault(false), "", Protection::Public, 0);
  }
} // class property

void Expression::enterClass_constraint(SysVerilogParser::Class_constraintContext *ctx)
{
  int         s = ctx->getStart()->getLine();
  int         t = ctx->getStop()->getLine();
  std::string name, type;
  if (ctx->constraint_declaration())
  {
    SVP::Constraint_declarationContext *cd = ctx->constraint_declaration();
    if (cd->T_STATIC())
      type = "static ";
    type.append("constraint");
    name = cd->ss()->getText();
    outlineParser()->addMember(name.data(), "", type.data(), s, t, EntryType::makeVariable(), TypeSpecifier().setDefault(false), "", Protection::Public, 0);
    return;
  }

  SVP::Constraint_prototypeContext *cp = ctx->constraint_prototype();
  if (cp->constraint_prototype_qualifier())
    type = cp->constraint_prototype_qualifier()->getText();

  if (cp->T_STATIC())
    type.append(" static ");

  type.append(" constraint");
  name = cp->ss()->getText();

  outlineParser()->addMember(name.data(), "", type.data(), s, s, EntryType::makeVariable(), TypeSpecifier().setDefault(false), "", Protection::Public, 0);
}

void Expression::enterCovergroup_declaration(SysVerilogParser::Covergroup_declarationContext *ctx)
{
  std::string event, id;
  int         s = ctx->getStart()->getLine();
  int         t = ctx->getStop()->getLine();

  if (ctx->tf_port_list())
  {
    event   = ("(");
    //uint st = ctx->tf_port_list()->getStart()->getTokenIndex();
    //uint e  = ctx->tf_port_list()->getStop()->getTokenIndex();
    event += vec2string(ctx->tf_port_list(), com);
    event += ")";
  }

  id = ctx->ss(0)->getText();

  outlineParser()->addMember(id.data(), event.data(), "covergroup", s, t, EntryType::makeVariable(), TypeSpecifier().setDefault(false), "", Protection::Public, "covergroup");
}

void Expression::exitCovergroup_declaration(SysVerilogParser::Covergroup_declarationContext *ctx)
{
  // m_sharedState->delClassEntry();
}

void Expression::enterClass_method(SysVerilogParser::Class_methodContext * ctx)
 {
  Token      *s     = ctx->getStart();
  Token      *t     = ctx->getStop();
  int         line  = s->getLine();
  int         eline = t->getLine();
  std::string name,arg,cn,xx,yy;

   bool b=checkRule(ctx->parent->parent,SVP::RuleClass_declaration);
   if(b)
   {
    ParseTree *p1=ctx->parent->parent->children.at(2);
    ParseTree *pt=ctx->parent->parent->children.at(1);
    cn=pt->getText();
    if(cn=="class")
       cn=p1->getText();
   }
      // SVP::Class_constructor_prototypeContext *cp = vm->class_constructor_prototype();
      SVP::Class_constructor_declarationContext *cp = ctx->class_constructor_declaration();
      SVP::Class_constructor_prototypeContext   *pr = ctx->class_constructor_prototype();

      if (cp)
      {
        if (cp->class_scope())
          name = cp->class_scope()->getText();
        else
          name = cn;

        QCString arg;
        if (cp->tf_port_list())
        {
          SVP::Tf_port_listContext *tpf = cp->tf_port_list();
          arg                           = parse_tf_port_item1(tpf, m_sharedState);
        }
       
        outlineParser()->addMember(name.data(), arg.data(), "", line,eline, EntryType::makeFunction(), TypeSpecifier().setDefault(true), "", Protection::Protected, "[constructor]",true);
        return;
      } // if

      if (pr)
      {
        QCString                                    arg, dim;
        std::string                                 func = "[constructor] extern ";
        std::vector<SVP::Method_qualifierContext *> vv0  = ctx->method_qualifier();

        name                                             = cn;
        if (!vv0.empty())
          dim = std::for_each(vv0.cbegin(), vv0.cend(), Sum()).sum;

        func.append(dim.data());
        func.append(" function ");
        if (pr->tf_port_list())
        {
          SVP::Tf_port_listContext *tpf = pr->tf_port_list();
          arg                           = parse_tf_port_item1(tpf, m_sharedState);
        }
        outlineParser()->addMember(name.data(), arg.data(), func.data(), line, eline, EntryType::makeFunction(), TypeSpecifier().setDefault(false), "", Protection::Protected, "function", true);
      }
 
  } // class_method
 

void Expression::enterClass_declaration(SysVerilogParser::Class_declarationContext *ctx)
{
  Token      *s     = ctx->getStart();
  Token      *t     = ctx->getStop();
  int         line  = s->getLine();
  int         eline = t->getLine();

  std::string cn;
  if (ctx->ss(0))
  {
    cn = ctx->ss(0)->getText();
  }

  std::string                           name;
  std::string                           life   = (ctx->lifetime() != NULL) ? ctx->lifetime()->getText() : "";
  std::string                           virt   = (ctx->T_VIRTUAL() != NULL) ? "virtual" : "";
  bool                                  ex     = (ctx->T_EXTENDS() != NULL);
  bool                                  interf = (ctx->T_IMPLEMENTS() != NULL);

  std::vector<SVP::Class_itemContext *> cu     = ctx->class_item();


  outlineParser()->addClass(cn.c_str(), line, eline, EntryType::makeClass(), TypeSpecifier().setValue(true), Protection::Public, "class");

  if (ex)
  {
    QCString               exts = ctx->class_type()->psid()->getText();
    std::shared_ptr<Entry> es   = outlineParser()->getLastEntity();
    es->extends.push_back(BaseInfo(exts, Protection::Private, Specifier::Pure));
  }

  if (interf)
  {
    std::shared_ptr<Entry>                          es  = outlineParser()->getLastEntity();
    std::vector<SVP::Interface_class_typeContext *> ict = ctx->interface_class_type();
    for (SVP::Interface_class_typeContext *io : ict)
    {
      QCString exts = io->psid()->getText();
      es->extends.push_back(BaseInfo(exts, Protection::Protected, Specifier::Pure));
    }
  }


#if 0
  for (SVP::Class_itemContext *cc : cu)
  {
    SVP::Class_methodContext *vm = cc->class_method();
    if (vm)
    {
      // SVP::Class_constructor_prototypeContext *cp = vm->class_constructor_prototype();
      std::cout<<"class method get start:"<<vm->getStart()->getLine()<<"\n";
      SVP::Class_constructor_declarationContext *cp = vm->class_constructor_declaration();
      SVP::Class_constructor_prototypeContext   *pr = vm->class_constructor_prototype();

      if (cp)
      {
        if (cp->class_scope())
          name = cp->class_scope()->getText();
        else
          name = cn;

        QCString arg;
        if (cp->tf_port_list())
        {
          SVP::Tf_port_listContext *tpf = cp->tf_port_list();
          arg                           = parse_tf_port_item1(tpf, m_sharedState);
        }
        // outlineParser()->addVerilogType(name.c_str(), s->getLine(), s->getLine(), EntryType::makeFunction(), TypeSpecifier().setFinal(true), true, Protection::Protected);
        name.append("1");
    
        outlineParser()->addMember(name.data(), arg.data(), " function void ", cp->getStart()->getLine(), cp->getStop()->getLine(), EntryType::makeFunction(), TypeSpecifier().setDefault(true), "", Protection::Public, "function",true);
        return;
      } // if

      if (pr)
      {
        QCString                                    arg, dim;
        std::string                                 func = "[constructor] extern ";
        std::vector<SVP::Method_qualifierContext *> vv0  = vm->method_qualifier();

        name                                             = cn;
        if (!vv0.empty())
          dim = std::for_each(vv0.cbegin(), vv0.cend(), Sum()).sum;

        func.append(dim.data());
        func.append(" function ");
        if (pr->tf_port_list())
        {
          SVP::Tf_port_listContext *tpf = pr->tf_port_list();
          arg                           = parse_tf_port_item1(tpf, m_sharedState);
        }
        outlineParser()->addMember(name.data(), arg.data(), func.data(), pr->getStart()->getLine(), pr->getStop()->getLine(), EntryType::makeFunction(), TypeSpecifier().setDefault(false), "", Protection::Protected, "function", true);
      }
    } // if class_item
  } // for
  #endif
}

void Expression::exitClass_declaration(SysVerilogParser::Class_declarationContext *ctx)
{
  m_sharedState->delClassEntry();
}

// --------- MODULE -----------------

void Expression::enterModule_declaration(SysVerilogParser::Module_declarationContext *ctx)
{
  std::vector<ParseTree *>                        v  = ctx->children;
  SysVerilogParser::Module_nonansi_headerContext *mo = ctx->module_nonansi_header();
  std::string                                     modName;

  Token                                          *s = ctx->getStart();
  Token                                          *t = ctx->getStop();



  if(mo!=nullptr && !mo->package_import_declaration().empty())
  {
    std::vector<SVP::Package_import_declarationContext*> vec=mo->package_import_declaration();
    size_t i=0;
    for(SVP::Package_import_declarationContext* pi:vec)
    {
      std::string im=pi->package_import_item(0)->getText();
      size_t len=im.find(":");
      std::string pack=im.substr(0,len);
      size_t line=pi->getStart()->getLine();
      outlineParser()->addMember(pack.data(), "", "import", line, line, EntryType::makeFunction(), TypeSpecifier().setValue(true), "", Protection::Package, 0, true);
 
    
    }
  }

  if (mo == nullptr)
  {

    if (ctx->ss(0))
      modName = ctx->ss(0)->getText();
    else
      modName="ZZZ";

    outlineParser()->addClass(modName.c_str(), s->getLine(), t->getLine(), EntryType::makeClass(), TypeSpecifier().setModule(true), Protection::Public, "module");
    return;
  }

  SysVerilogParser::LifetimeContext *lif = mo->lifetime();

  modName                                = mo->ss()->getText();

  outlineParser()->addClass(modName.c_str(), s->getLine(), t->getLine(), EntryType::makeClass(), TypeSpecifier().setModule(true), Protection::Public, "module");
}

void Expression::exitModule_declaration(SysVerilogParser::Module_declarationContext *ctx)
{
  m_sharedState->delClassEntry();
}

// -------- SUBROUTINES FUNCTION / TASK -  parameter operation_mode                       = "normal";

void Expression::enterEvent_expression(SysVerilogParser::Event_expressionContext *ctx)
{
  antlr4::misc::Interval                      iv  = ctx->getSourceInterval();
  size_t                                      w   = iv.length();
  //int                                         i   = ctx->getStart()->getTokenIndex();
  //int                                         j   = ctx->getStop()->getTokenIndex();
  std::string                                 res = vec2string(ctx, com);

  std::shared_ptr<Entry>                      se  = outlineParser()->getLastEntry();
  
  if(!se->section.isFunction())
    return;
  
  std::vector<SVP::Event_expressionContext *> ev  = ctx->event_expression();
  for (SVP::Event_expressionContext *e : ev)
  {
    for (SVP::ExpressionContext *ep : e->expression())
    {

      struct Argument ar;
      if (ep->expression(0))
      {
        continue;
      }
      ar.name = ep->getText().data();
      se->argList.push_back(ar);
    }
  }
  if (se->exception.isEmpty())
  {
    se->exception = " ";
    se->args.append(" (");
    se->args.append(res);
    se->args.append(")");
  }
}

void Expression::enterAlways_construct(SysVerilogParser::Always_constructContext *ctx)
{
  std::string idd, keyw, pname;
  Token      *s               = ctx->getStart();
  Token      *t               = ctx->getStop();

  keyw                        = ctx->always_keyword()->getText();
  QCString               name = "process" + getProcessNumber();
  SVP::StatementContext* sc=ctx->statement();

  if(sc && sc->statement_item())
  {
    SVP::Statement_itemContext* sic=sc->statement_item(); 
    if(sic->seq_block() && sic->seq_block()->ss(0))
      name=sic->seq_block()->ss(0)->getText();

    if(sic->procedural_timing_control_statement() && sic->procedural_timing_control_statement()->statement_or_null())
    {
     SVP::Statement_or_nullContext* sn = sic->procedural_timing_control_statement()->statement_or_null();
     if(sn->statement() && sn->statement()->statement_item())
     {
       SVP::Statement_itemContext* ssic=sn->statement()->statement_item();
        if(ssic->seq_block() && ssic->seq_block()->ss(0))
          name=ssic->seq_block()->ss(0)->getText();
      }
    }
  }

  // if(sic->procedural_timing_control_statement())
  // {} && sic->procedural_timing_control_statement()->)
  
  SVP::StatementContext *ssv  = ctx->statement();
  if (ssv && ssv->statement_item())
    if (ssv->statement_item()->seq_block())
    {
      SVP::SsContext *sd = ssv->statement_item()->seq_block()->ss(0);
      if (sd)
        idd = sd->getText();
    }

  if (idd.empty())
    idd = name.data();

  outlineParser()->addMember(idd.data(), keyw.data(), "", s->getLine(), t->getLine(), EntryType::makeFunction(), TypeSpecifier().setFinal(false), "", Protection::Public, "process");
}

void Expression::enterMethod_prototype(SysVerilogParser::Method_prototypeContext *ctq)
{
  std::string                     name, dat, func;
  bool                            virt;

  Token                          *s   = ctq->getStart();
  Token                          *t   = ctq->getStop();

  SVP::Function_prototypeContext *ctx = ctq->function_prototype();
  SVP::Task_prototypeContext     *tcx = ctq->task_prototype();
  if (ctx)
  {
    getMethodQualifier(ctx->parent->parent, func);
    m_sharedState->isProto = (func.find("extern ") != std::string::npos);
    func.append("function ");

    if (ctx->data_type_or_void())
    {
      parseDataOrVoid(ctx->data_type_or_void(), dat);
      func.append(dat);
    }

    name = ctx->ss()->getText();
    QCString arg;
    if (ctx->tf_port_list())
    {
      arg = parse_tf_port_item1(ctx->tf_port_list(), m_sharedState);
    }
    // std::cout<<"method proto:"<<name.data()<<"\n";
    outlineParser()->addMember(name.data(), arg.data(), func.data(), s->getLine(), t->getLine(), EntryType::makeFunction(), TypeSpecifier().setAbstract(virt), "", Protection::Public, 0, true);
  } // function_proto

  if (tcx)
  {
    getMethodQualifier(tcx->parent->parent, func);
    func.append(" task ");

    name = tcx->ss()->getText();
    QCString arg;
    if (tcx->tf_port_list())
    {
      arg = parse_tf_port_item1(tcx->tf_port_list(), m_sharedState);
    }

    outlineParser()->addMember(name.data(), arg.data(), func.data(), s->getLine(), t->getLine(), EntryType::makeFunction(), TypeSpecifier().setAbstract(virt), "", Protection::Public, 0, true);
  }
}

void Expression::enterTask_declaration(SysVerilogParser::Task_declarationContext *ctx)
{
  std::string                        funcName, type;
  QCString                           arg;
  std::string                        clscope;
  Token                             *s   = ctx->getStart();
  size_t                             po  = s->getLine();

  Token                             *t   = ctx->getStop();
  size_t                             ioo = t->getLine();
  SVP::LifetimeContext              *ltc = ctx->lifetime();
  SVP::Task_body_declarationContext *tbd = ctx->task_body_declaration();
  SVP::Tf_port_listContext          *tpl = tbd->task_decl()->tf_port_list();

   getMethodQualifier(ctx->parent,type);
    type.append(" task ");

  if (ltc)
    type.append(ltc->getText());

  if (tbd->class_scope())
  {
    clscope = tbd->class_scope()->getText();
  }

  if (tbd->DOT())
    funcName = tbd->ss(1)->getText();
  else
    funcName = tbd->ss(0)->getText();

  if (tpl)
  {
    arg = parse_tf_port_item1(tpl, m_sharedState);
  }

  outlineParser()->addMember(funcName.data(), arg.data(), type.data(), s->getLine(), t->getLine(), EntryType::makeFunction(), TypeSpecifier().setValue(true), "", Protection::Public, "task", true);
}

void Expression::enterFunction_declaration(SysVerilogParser::Function_declarationContext *ctx)
{
  std::string                                                 funcName, reti, stat, fu;
  std::string                                                 clscope;
  Token                                                      *s    = ctx->getStart();
  size_t                                                      lii  = s->getLine();
  Token                                                      *t    = ctx->getStop();
  SysVerilogParser::LifetimeContext                          *ltc  = ctx->lifetime();
  SysVerilogParser::Function_body_declarationContext         *fbd  = ctx->function_body_declaration();
  SysVerilogParser::Function_decContext                      *fdc  = fbd->function_dec();
  SysVerilogParser::Tf_port_listContext                      *tpl  = fdc->tf_port_list();
  std::vector<SysVerilogParser::Tf_item_declarationContext *> tpi  = fdc->tf_item_declaration();
  std::vector<SysVerilogParser::SsContext *>                  op   = fbd->ss();
  SysVerilogParser::Function_data_type_or_implicitContext    *fdii = fbd->function_data_type_or_implicit();


  getMethodQualifier(ctx->parent,fu);
   fu += "function ";

   if (ltc && ltc->T_AUTOMATIC())
    fu += "automatic ";

  if (fdii)
  {
    fu+=vec2string(fdii,com);
  }

  if (tpl)
  {
    parse_tf_port_item(tpl, m_sharedState);
  }
  else
    parse_tf_port_declaration(tpi, m_sharedState);


#if 0
  if (ltc && ltc->T_STATIC())
    fu += "static ";

 
#endif
  if (fbd->class_scope())
  {
    funcName.append(fbd->ss(0)->getText());
  }
  else if (fbd->DOT())
  {
    funcName = fbd->ss(0)->getText();
    funcName.append(".");
    funcName.append(fbd->ss(1)->getText());
  }
  else if (fbd->ss(0))
    funcName = fbd->ss(0)->getText();
  else
    funcName = fdii->getText();



  // outlineParser()->addMember(funcName.data(), arg.data(), type.data(), s->getLine(), t->getLine(), EntryType::makeFunction(), TypeSpecifier().setValue(true), "", Protection::Public, "task", true);
  outlineParser()->addMember(funcName.data(), m_sharedState->current->args.data(), fu.data(), s->getLine(), t->getLine(), EntryType::makeFunction(), TypeSpecifier().setValue(true), "", Protection::Public, "function", true);
}

// --------------- STRUCT / UNION

void Expression::enterStruct_union_member(SysVerilogParser::Struct_union_memberContext *ctx)
{
  std::string r, s, t, args;
  Token      *tok = ctx->getStart();
 // checkChildren(ctx->parent);
  if(ctx->parent->getText()=="struct")
    return;

std::list<antlr4::RuleContext *> li;
getRules(ctx->parent,li,SVP::RuleStruct_union);

for(antlr4::RuleContext * rlc:li)
{

   //std::cout << " inner struct member:" << s << " " << rlc->getText()  << "\n";
}

  if (ctx->random_qualifier())
  {
    s = ctx->random_qualifier()->getText();
    s.append(" ");
  }

  int                            l  = ctx->getStart()->getLine();
  int                            lwi  = ctx->getStop()->getLine();
  
  
  SVP::Data_type_or_voidContext *cc = ctx->data_type_or_void();

  bool                           b  = parseDataOrVoid(cc, r);
  s.append(r);
  if(s.find("struct")!=std::string::npos)
   return;
  SVP::List_of_variable_decl_assignmentsContext      *ld  = ctx->list_of_variable_decl_assignments();
  std::vector<SVP::Variable_decl_assignmentContext *> lop = ld->variable_decl_assignment();
  for (SVP::Variable_decl_assignmentContext *vp : lop)
  {
    int ll = 0;
    parseVariableDeclAssignment(b, t, args, vp, ll);
   //  std::cout << " found struct member:" << s << " | " << args << " | " << t << "\n";
    outlineParser()->addMember(t.data(), args.data(), s.data(), l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Public, "data");
  }
}

void Expression::enterStruct_union(SysVerilogParser::Struct_unionContext *ctx)
{
 std::string qualifier, s, t, args;
 Token      *tok = ctx->getStart();
 size_t line=tok->getLine();
 //std::cout<<"enter struct "<<" "<<tok->getLine()<<"\n";

 qualifier=ctx->getText();
 std::list<antlr4::RuleContext *> dat;
getRules(ctx->parent->parent->parent,dat,SVP::RuleSs);

antlr4::ParserRuleContext* pr4=static_cast<antlr4::ParserRuleContext*>(dat.back());
size_t endLine=pr4->getStop()->getLine();
std::string id=dat.back()->getText();
TypeSpecifier tsp;
 if (qualifier.compare("struct") == 0)
      tsp.setStruct(true);
    else
      tsp.setUnion(true);

outlineParser()->addClass(id.c_str(), line, line, endLine, EntryType::makeClass(), tsp, Protection::Public, qualifier.data());
   

}


void Expression::visitErrorNode(antlr4::tree::ErrorNode *node)
{
  size_t li = node->getSymbol()->getLine();
  std::cerr << "\n\n error while parsing file:" << fileName << " at line:" << li << " [" << node->getText() << "] \n";
  outlineParser()->removeLastEntry(li);
  // exit(20);
}

void Expression::enterNet_declaration(SysVerilogParser::Net_declarationContext *ctx)
{
  Token                                         *tok = ctx->getStart();
  int                                            l   = tok->getLine();
  SVP::List_of_net_decl_assignmentsContext      *la  = ctx->list_of_net_decl_assignments();
  std::vector<SVP::Net_decl_assignmentContext *> vec;
  if (la)
    vec = la->net_decl_assignment();
  std::string name, expr, ntype, ooo;
  if (ctx->net_type())
    ntype = ctx->net_type()->getText() + " ";
  // [ drive_strength | charge_strength ] [ vectored | scalared ] data_type_or_implicit
  bool b = ctx->data_type_or_implicit();

  if (ctx->drive_strength())
    ntype += ctx->drive_strength()->getText() + " ";
  if (ctx->charge_strength())
    ntype.append(ctx->charge_strength()->getText() + " ");
  if (ctx->T_SCALARED())
    ntype.append("scalared ");
  if (ctx->T_VECTORED())
    ntype.append("vectored ");

  if (!b)
  {
    if (!ctx->ss().empty())
    {
      ooo = ctx->ss(0)->getText();
      ntype.append(ooo + " ");
    }
  }

  if (b)
    ntype.append(ctx->data_type_or_implicit()->getText());

  for (SVP::Net_decl_assignmentContext *net : vec)
  {
    name = net->ss()->getText();
    if (net->EQ())
      expr = net->expression()->getText();

    outlineParser()->addMember(name.data(), expr.data(), ntype.data(), l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Protected, "Net");
  }
}

void Expression::enterModule_instantiation(SVP::Module_instantiationContext *ctx)
{
  Token                                           *tok = ctx->getStart();
  int                                              l   = tok->getLine();
  std::string                                      name, hier, temp;
  std::vector<SVP::Hierarchical_instanceContext *> hc = ctx->hierarchical_instance();
  name                                                = ctx->ss()->getText();
  for (SVP::Hierarchical_instanceContext *hi : hc)
  {
    hier = hi->name_of_instance()->ss()->getText();
    outlineParser()->addMember(hier.data(), "", name.data(), l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "", Protection::Package, "module instance");
  }
}

void Expression::enterNamed_port_connection(SysVerilogParser::Named_port_connectionContext *ctx)
{
  Token      *tok = ctx->getStart();
  std::string name;
  if (ctx->ss())
    name = ctx->ss()->getText();

  // std::cout << "named port:" << name << "\n";
}

void Expression::enterInclude_path(SysVerilogParser::Include_pathContext *ctx)
{
  Token      *tok = ctx->getStart();
  int         l   = tok->getLine();
  size_t      top = tok->getType();
  std::string inc = ctx->String_literal()->getText();
  replaceChar(inc, "\"", "");
  std::string res = stripPath(QCString(inc)).data();

  bool        am;
  FileDef    *fd  = outlineParser()->getFileDef();
  // FileDef *fdd = findFileDef(Doxygen::, QCString(res.data()), am);
  FileDef    *fdd = findFileDef(Doxygen::inputNameLinkedMap, inc.c_str(), am);
  // std::string qc = showFileDefMatches(Doxygen::inputNameLinkedMap, QCString(inc)).data();
  if (fd)
  {
    fd->addIncludeDependency(fdd, inc.c_str(), IncludeKind::ImportModule);
  }
}

void Expression::enterLibrary_description(SysVerilogParser::Library_descriptionContext *ctx)
{
  Token      *tok = ctx->getStart();
  int         l   = tok->getLine();
  std::string name, hier, temp;
  if (ctx->T_INCLUDEDIR())
  {
    temp = ctx->String_literal(0)->getText();
    replaceChar(temp, "\"", "");
    // res=getFileFromPath(temp);
    std::string res = stripPath(QCString(temp)).data();

    FileDef    *fd  = outlineParser()->getFileDef();
    std::string vl  = fd->getDefFileName().data();
    QCString    oo  = res.data();
    bool        am;
    // FileDef *fdd = findFileDef(Doxygen::, QCString(res.data()), am);
    FileDef    *fdd = findFileDef(Doxygen::inputNameLinkedMap, res.c_str(), am);
    std::string qcc = showFileDefMatches(Doxygen::inputNameLinkedMap, oo).data();
    std::string sp  = findFilePath(oo, am).data();

    if (fd)
    {
      fd->addIncludeDependency(NULL, QCString(res), IncludeKind::ImportModule);
    }

    //  QCString name = getProcessNumber();
    //  outlineParser()->addMember(res.data(), "include11", res.data(), l, l, EntryType::makeInclude(), TypeSpecifier().setValue(true), "", Protection::Protected, "include");
  }
}

void Expression::enterTimeunits_declaration(SysVerilogParser::Timeunits_declarationContext *ctx)
{
  Token                                  *tok = ctx->getStart();
  int                                     l   = tok->getLine();
  std::string                             sl  = tok->getText();
  bool                                    dd  = ctx->T_TIMEUNIT();
  bool                                    bb  = ctx->T_TIMEPRECISION();
  std::vector<SVP::Time_literalContext *> v   = ctx->time_literal();
  assert(v.size() > 0);
  std::string t     = "t";
  uint        l1    = v.at(0)->getStart()->getLine();
  std::string first = v.at(0)->getText();
  std::string s1    = t + getProcessNumber().data();
  std::string uni, prec;
  if (v.size() == 2)
  {
    uint        l2  = v.at(1)->getStart()->getLine();
    std::string sec = v.at(1)->getText();
    std::string s2  = t + getProcessNumber().data();
    if (dd && sl.compare("timeunit") == 0)
    {
      uni  = "timeunit:";
      prec = "timeprecision:";
    }
    else
    {
      prec = "timeunit:";
      uni  = "timeprecision:";
    }

    outlineParser()->addMember(s1.data(), "", first.data(), l1, l1, EntryType::makeVariable(), TypeSpecifier().setValue(true), uni.data(), Protection::Protected, uni.data());
    outlineParser()->addMember(s2.data(), "", sec.data(), l2, l2, EntryType::makeVariable(), TypeSpecifier().setValue(true), prec.data(), Protection::Protected, prec.data());
  }
  else if (bb)
    outlineParser()->addMember(s1.data(), "", first.data(), l1, l1, EntryType::makeVariable(), TypeSpecifier().setValue(true), "timeprecision:", Protection::Protected, "timeprecision");
  else if (dd)
    outlineParser()->addMember(s1.data(), "", first.data(), l1, l1, EntryType::makeVariable(), TypeSpecifier().setValue(true), "timeunit:", Protection::Protected, "timeunit");
}

void Expression::enterTime_literal(SysVerilogParser::Time_literalContext *ctx)
{
}


void Expression::enterEveryRule(antlr4::ParserRuleContext *ctx)
{
}

void Expression::visitTerminal(antlr4::tree::TerminalNode *node)
{
  Token      *tok = node->getSymbol();
  std::string s   = tok->getText();
  // std::cout<<"terminal: "<<s<<" beginline:"<<tok->getLine()<<"\n";
  parseHiddenTokens(tok);
}

Expression::Expression(antlr4::CommonTokenStream *cs, SysVerilogParser *ex, SharedState *vo) : com(cs), exe(ex), m_sharedState(vo)
{
  this->fileName = vo->fileName;
}

void Expression::enterDpi_import_export(SysVerilogParser::Dpi_import_exportContext *ctx)
{
  std::string                               ret, imp, name, func, proto;
  Token                                    *tok        = ctx->getStart();
  int                                       l          = tok->getLine();
  std::string                               sl         = tok->getText();
  TerminalNode                             *b1         = ctx->T_EXPORT();
  TerminalNode                             *b2         = ctx->T_IMPORT();
  TerminalNode                             *f1         = ctx->T_FUNCTION();
  TerminalNode                             *f2         = ctx->T_TASK();

  SVP::Dpi_function_import_propertyContext *dpic       = ctx->dpi_function_import_property();
  SVP::Dpi_function_protoContext           *dpifuncpro = ctx->dpi_function_proto();
  SVP::Dpi_task_protoContext               *dpitaskpro = ctx->dpi_task_proto();


  std::list<antlr4::RuleContext *>          li, dat;
  getRules(ctx->parent, li, SVP::RuleSs);
  getRules(ctx->parent, dat, SVP::RuleData_type_or_void);
  


  if (!dat.empty())
  {
   antlr4::ParserRuleContext *pp=static_cast<antlr4::ParserRuleContext*>(dat.front());
   ret = vec2string(pp,com); //->getText();
  }
  
  if (!li.empty())
  {
    SVP::SsContext *di = static_cast<SVP::SsContext *>(li.front());
    if (di)
      name = di->getText();
  }


  if (b2)
    imp = "import \"DPI-C\" ";
  else
    imp = "export \"DPI-C\" ";

  if (f2)
    func = " task ";
  else
    func = ret + " function ";

  if (dpifuncpro)
  {
    // proto=dpifuncpro->getText();
    SVP::Tf_port_listContext *tf;
    if (dpifuncpro->function_prototype())
      if (dpifuncpro->function_prototype()->tf_port_list())
      {
        SVP::Tf_port_listContext *tf = dpifuncpro->function_prototype()->tf_port_list();
       // int                       i  = tf->getStart()->getTokenIndex();
       // int                       j  = tf->getStop()->getTokenIndex();
        proto                        = "(";
        proto += vec2string(tf, com);
        proto += ")";
      }
  }


  if (dpitaskpro)
    proto = dpitaskpro->getText();

  imp += func;

  outlineParser()->addMember(name.data(), proto.data(), imp.data(), l, l, EntryType::makeFunction(), TypeSpecifier().setValue(true), "", Protection::Private, "DPI-C",true);
}

void Expression::enterModport_declaration(SysVerilogParser::Modport_declarationContext *ctx)
{
  Token *tok = ctx->getStart();
  int l  = tok->getLine();
  
 // std::cout<<"modport:"<< ctx->getText()<<"\n";
  SVP::Modport_itemContext* mod=ctx->modport_item(0);
  assert(mod);
  std::string name=mod->ss()->getText();
  std::vector<SVP::Modport_ports_declarationContext*> vm=mod->modport_ports_declaration(); 
  
  //QCString f = std::for_each(vm.cbegin(), vm.cend(), Sum()).sum;

  std::string f="(";
  size_t len=0; //=vm.size();
  for(SVP::Modport_ports_declarationContext* pdec:vm)
  {
    f.append(vec2string(pdec,com).data());
    if(++len<vm.size())
    f.append(",");
  }
  f.append(")");
  //std::string f=vec2string(mod->modport_ports_declaration(0),com);

   outlineParser()->addMember(name.data(), "", f.data(), l, l, EntryType::makeVariable(), TypeSpecifier().setValue(true), "modport", Protection::Protected, "modport",true);

}
