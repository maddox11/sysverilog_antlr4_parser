

#ifndef PREPARSER_H
#define PREPARSER_H
#include "Token.h"
#include <string>
#include <iostream>
#include <list>
#include <map>
#include <fstream>
#include "BitS.h"
#include "filedef.h"
#include "mutex"
#include <regex>
#include "textstream.h"
#include <iostream>
#include <shared_mutex>
#include "sharedstate.h"
// using namespace std;
class DefMacro;
template <class DefMacro> class ConcurrentMacroMap;
struct SharedState;

namespace prepc
{

   typedef std::map<std::string, std::shared_ptr<DefMacro>> DefVector;

   class IncludeList
   {
   private:
      std::list<std::string> m_list;

   public:
      void insert(std::string &item)
      {
         m_list.emplace_back(item);
      }

      std::string find(std::string &str)
      {

         for (std::string path : m_list)
         {
           // if (QCString(path).endsWith(str.c_str()))
               return path;
         }
         return "";
      }
   };

  
   struct DefCont
   {
      std::string image;
      std::string ifdef;
      int line;
      bool parse;
      int level;

      ~DefCont() { /* std::cout<<"\n deleting: "<<ifdef<<" "<<image<<"\n";*/ }
      DefCont(const std::string &im, const std::string &s, int l, bool b, int lev);
      void debug();
   };

   class InsertDefinition
   {

   public:
      std::vector<std::string> al;
      DefMacro *d;

      InsertDefinition(std::list<std::string> &all, DefMacro *dd)
      {
         this->al.insert(al.begin(), all.begin(), all.end());
         this->d = dd;
      }

      std::string parseValue(std::string s, ConcurrentMacroMap<DefMacro> &);
      std::string getValue(std::string s);
   };

   
   class PrepParser
   {
   public:
      PrepParser();
      PrepParser(const std::string &f);
      PrepParser(bool b,SharedState *sh);
      PrepParser(const std::string &f, SharedState *sh);
      void _clearAll();
      ~PrepParser()
      {
         for (auto it = ifMap.begin(); it != ifMap.end(); it++)
            delete *it;
      }

      static void clearConcMap();

      void setBitField(const std::string &s);
      std::string parseData(const std::string &file);
      std::string getIncludeFile(const std::string &f);
    
      std::list<std::string> getFuncArguments(const std::string &s);
      DefVector getMacros();
      void writeMacros();
      void appStringDef(const Token* tok,size_t kind);
      void writeOutput(const char* file, const std::string &text);
      void readFile(const std::string &fi, std::string &buffer);
      bool checkIf(size_t kind);
      bool getPrevLevel(int l);
      void parseMacro(const Token *t, DefMacro *dem = NULL);
      std::list<std::string> getArguments(std::string &t);
      bool getParserState(int l);
      bool getParserStateElif(int l);
      void adjust(int l);
      bool valIfdef(std::string ifc, const char* image, int line, bool flag, int l);
      int countLines(const Token *tok, bool in);
      int countLines(const std::string &tok, bool in, int s, int e);
      int countChar(const std::string &s, char c);
      // void checkString(Token *t);
      void appNewLines(int n);
      void appStringSpecial(const std::string &str);
      void appString0(const Token *tok, bool ins);
      void appString(const std::string&, bool ins);
      void appString(const Token *tok);
      void appString(const std::string str);
      void appStringSkip(const Token *t);
      void appStringSkip(const char *c);
      void appSysMacros(const Token *t);

      //----------------------------
      void endif(const Token *tok);
      void ifdef(const Token *tok);
      void ifndef(const Token *tok);
      void _else(const Token *tok);
      void elsif(const Token *tok);
      void define(const Token *tok);
      void undef(const Token *tok);
      //----------------------------

      void macro(const Token *tok, bool b);
      void include(const Token *tok);
      void parseComment(Token *tok);
      void printStack();
      std::string insertDelLines(std::string sb, PrepParser *p);
      //----------------------------
      std::string getMName(const std::string &s);
      // bool checkM(std::string reg, std::string m);
      int getProto(const std::string &m, int x);
      std::string evalMacro(std::string s1, bool ins, int beginLine);
      std::string match(std::string mac, const std::regex &r, InsertDefinition *gn);
      std::string parseDefinition(std::string m);
      bool checkInline(prepc::Token *tok);
      //  void includeDir();
      std::string updateMacro(DefMacro *def);
      std::string insertMacroName(std::string s, PrepParser *p);
      void setPredef();
      bool findExpandAs(const std::string &def, bool *);
    
      void replace(std::string &str, const char *suf, const char *ref);
      //void replace(QCString &str, const char *suf, const char *ref);
      void trim(std::string &s);
      int line = 0;
      int level = 0;
      int z_old = 0;
      bool pMM = false;
      bool parserOn = false;
      SharedState *m_shared = NULL;
      std::string sbb;
      std::string buf;
      std::vector<DefCont *> ifMap;
      BitS bit;

   private:
      std::string m_file;
      bool m_psource;
      //  std::map<std::string, DefMacro *> dma;

   }; // Prepparser

   void writeOutput(const char* file, const std::string &text);
     
   void searchDirectories();
   // sentinal for outline source code `ifdef .... `endif
   const std::string OUTL = "@@";
   // bool endsWith(const std::string &str, const std::string &suffix);
   void do_something(std::string &v);
} // prepc

std::string eraseComment(std::string &line);

#endif
