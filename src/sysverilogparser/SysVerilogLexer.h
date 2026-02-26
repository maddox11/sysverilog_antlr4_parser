
// Generated from SysVerilogLexer.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"




class  SysVerilogLexer : public antlr4::Lexer {
public:
  enum {
    Macro_directive = 1, LP = 2, RP = 3, LC = 4, RC = 5, LB = 6, RB = 7, 
    SEMI = 8, COMMA = 9, DOT = 10, COLONCOLON = 11, ATSTAR = 12, COLON = 13, 
    MINUS = 14, STAR = 15, MINUSMINS = 16, NOT = 17, NE = 18, NEQ = 19, 
    NEE = 20, DPI = 21, DPIC = 22, P = 23, PP = 24, PMP = 25, PEP = 26, 
    DOLLAR = 27, DERROR = 28, DFATAL = 29, DINFO = 30, DROOT = 31, DUNIT = 32, 
    DWARNING = 33, PER = 34, PE = 35, AND = 36, ANDAND = 37, ANDANDAND = 38, 
    AE = 39, SS = 40, SCCS = 41, SEQ = 42, SGT = 43, DOTSTAR = 44, SLASH = 45, 
    SLASHEQ = 46, MCOLON = 47, COLONSLASH = 48, COLONEQ = 49, QUES = 50, 
    AT = 51, ATAT = 52, QUOTE = 53, CARET = 54, CARETSQUIG = 55, CARETEQ = 56, 
    BAR = 57, BARBAR = 58, BAREQ = 59, BAREQGT = 60, BARARROW = 61, SQUIG = 62, 
    SQUIGAND = 63, SQUIGCARET = 64, SQUIGBAR = 65, PLUS = 66, PLUSCOLON = 67, 
    PLUSPLUS = 68, PLUSEQ = 69, LT = 70, LTLT = 71, LTLTLT = 72, LTLTLTEQ = 73, 
    LTLTEQ = 74, LTEQ = 75, LTMINUSGT = 76, EQ = 77, MINUSEQ = 78, EQEQ = 79, 
    EQEQQUEST = 80, EQEQEQ = 81, EQGT = 82, GT = 83, ARROW = 84, GE = 85, 
    GTGT = 86, MINUSGTGT = 87, GTGTEQ = 88, GTGTGT = 89, GTGTGTEQ = 90, 
    T_ONESTEP = 91, T_ACCEPT_ON = 92, T_ALIAS = 93, T_ALWAYS = 94, T_ALWAYS_COMB = 95, 
    T_ALWAYS_FF = 96, T_ALWAYS_LATCH = 97, T_AND = 98, T_ASSERT = 99, T_ASSIGN = 100, 
    T_ASSUME = 101, T_AUTOMATIC = 102, T_BEFORE = 103, T_BEGIN = 104, T_BIND = 105, 
    T_BINS = 106, T_BINSOF = 107, T_BIT = 108, T_BREAK = 109, T_BUF = 110, 
    T_BUFIF0 = 111, T_BUFIF1 = 112, T_BYTE = 113, T_CASE = 114, T_CASEX = 115, 
    T_CASEZ = 116, T_CELL = 117, T_CHANDLE = 118, T_CHECKER = 119, T_CLASS = 120, 
    T_CLOCKING = 121, T_CMOS = 122, T_CONFIG = 123, T_CONST = 124, T_CONSTRAINT = 125, 
    T_CONTEXT = 126, T_CONTINUE = 127, T_COVER = 128, T_COVERGROUP = 129, 
    T_COVERPOINT = 130, T_CROSS = 131, T_DEASSIGN = 132, T_DEFAULT = 133, 
    T_DEFPARAM = 134, T_DESIGN = 135, T_DISABLE = 136, T_DIST = 137, T_DO = 138, 
    T_EDGE = 139, T_ELSE = 140, T_END = 141, T_ENDCASE = 142, T_ENDCHECKER = 143, 
    T_ENDCLASS = 144, T_ENDCLOCKING = 145, T_ENDCONFIG = 146, T_ENDFUNCTION = 147, 
    T_ENDGENERATE = 148, T_ENDGROUP = 149, T_ENDPROGRAM = 150, T_ENDINTERFACE = 151, 
    T_ENDMODULE = 152, T_ENDPACKAGE = 153, T_ENDSEQUENCE = 154, T_ENDSPECIFY = 155, 
    T_ENDTABLE = 156, T_ENDTASK = 157, T_ENDPROPERTY = 158, T_ENDPRIMITIVE = 159, 
    T_ENUM = 160, T_EVENT = 161, T_EVENTUALLY = 162, T_EXPECT = 163, T_EXPORT = 164, 
    T_EXTENDS = 165, T_EXTERN = 166, T_FINAL = 167, T_FIRST_MATCH = 168, 
    T_FOR = 169, T_FORCE = 170, T_FOREACH = 171, T_FOREVER = 172, T_FORK = 173, 
    T_FORKJOIN = 174, T_FUNCTION = 175, T_GENERATE = 176, T_GENVAR = 177, 
    T_GLOBAL = 178, T_HIGHZ0 = 179, T_HIGHZ1 = 180, T_IF = 181, T_IFF = 182, 
    T_IFNONE = 183, T_IGNORE_BINS = 184, T_ILLEGAL_BINS = 185, T_IMPLEMENTS = 186, 
    T_IMPLIES = 187, T_IMPORT = 188, T_INCDIR = 189, T_INCLUDE = 190, T_INITIAL = 191, 
    T_INOUT = 192, T_INPUT = 193, T_INSIDE = 194, T_INSTANCE = 195, T_INTEGER = 196, 
    T_INT = 197, T_INTERCONNECT = 198, T_INTERFACE = 199, T_INTERSECT = 200, 
    T_JOIN = 201, T_JOIN_ANY = 202, T_JOIN_NONE = 203, T_LARGE = 204, T_LET = 205, 
    T_LIBLIST = 206, T_LIBRARY = 207, T_LOCAL = 208, T_LOCALPARAM = 209, 
    T_LOGIC = 210, T_LONGINT = 211, T_MACROMODULE = 212, T_MATCHES = 213, 
    T_MEDIUM = 214, T_MODPORT = 215, T_MODULE = 216, T_NAND = 217, T_NEGEDGE = 218, 
    T_NETTYPE = 219, T_NEW = 220, T_NEXTTIME = 221, T_NMOS = 222, T_NOR = 223, 
    T_NOSHOWCANCELLED = 224, T_NOT = 225, T_NOTIF0 = 226, T_NOTIF1 = 227, 
    T_NULL = 228, T_OPTION = 229, T_OR = 230, T_OUTPUT = 231, T_PACKAGE = 232, 
    T_PACKED = 233, T_PARAMETER = 234, T_PATHPULSE = 235, T_PMOS = 236, 
    T_POSEDGE = 237, T_PRIMITIVE = 238, T_PRIORITY = 239, T_PROGRAM = 240, 
    T_PROPERTY = 241, T_PROTECTED = 242, T_PULL0 = 243, T_PULL1 = 244, T_PULLDOWN = 245, 
    T_PULLUP = 246, T_PULSESTYLE_ONDETECT = 247, T_PULSESTYLE_ONEVENT = 248, 
    T_PURE = 249, T_RAND = 250, T_RANDC = 251, T_RANDCASE = 252, T_RANDOMIZE = 253, 
    T_RANDSEQUENCE = 254, T_RCMOS = 255, T_REAL = 256, T_REALTIME = 257, 
    T_REF = 258, T_REG = 259, T_REJECT_ON = 260, T_RELEASE = 261, T_REPEAT = 262, 
    T_RESTRICT = 263, T_RETURN = 264, T_RNMOS = 265, T_RPMOS = 266, T_RTRAN = 267, 
    T_RTRANIF0 = 268, T_RTRANIF1 = 269, T_S_ALWAYS = 270, T_S_EVENTUALLY = 271, 
    T_S_NEXTTIME = 272, T_S_UNTIL = 273, T_S_UNTIL_WITH = 274, T_SCALARED = 275, 
    T_SEQUENCE = 276, T_SHORTINT = 277, T_SHORTREAL = 278, T_SHOWCANCELLED = 279, 
    T_SIGNED = 280, T_SMALL = 281, T_SOFT = 282, T_SOLVE = 283, T_SPECIFY = 284, 
    T_SPECPARAM = 285, T_STATIC = 286, T_STRING = 287, T_STRONG = 288, T_STRONG0 = 289, 
    T_STRONG1 = 290, T_STRUCT = 291, T_SUPER = 292, T_SUPPLY0 = 293, T_SUPPLY1 = 294, 
    T_SYNC_ACCEPT_ON = 295, T_SYNC_REJECT_ON = 296, T_TAGGED = 297, T_TABLE = 298, 
    T_TASK = 299, T_THIS = 300, T_THROUGHOUT = 301, T_TIME = 302, T_TIMEPRECISION = 303, 
    T_TIMEUNIT = 304, T_TRAN = 305, T_TRANIF0 = 306, T_TRANIF1 = 307, T_TRI = 308, 
    T_TRI0 = 309, T_TRI1 = 310, T_TRIAND = 311, T_TRIOR = 312, T_TRIREG = 313, 
    T_TYPE = 314, T_TYPE_OPTION = 315, T_TYPEDEF = 316, T_UNION = 317, T_UNIQUE = 318, 
    T_UNIQUE0 = 319, T_UNSIGNED = 320, T_UNTIL = 321, T_UNTIL_WITH = 322, 
    T_UNTYPED = 323, T_USE = 324, T_UWIRE = 325, T_VAR = 326, T_VECTORED = 327, 
    T_VIRTUAL = 328, T_VOID = 329, T_WAIT = 330, T_WAIT_ORDER = 331, T_WAND = 332, 
    T_WEAK = 333, T_WEAK0 = 334, T_WEAK1 = 335, T_WHILE = 336, T_WILDCARD = 337, 
    T_WIRE = 338, T_WITH = 339, T_WITHIN = 340, T_WOR = 341, T_XNOR = 342, 
    T_XOR = 343, T_SETUP = 344, T_HOLD = 345, T_SETUPHOLD = 346, T_RECOVERY = 347, 
    T_RECREM = 348, T_SKEW = 349, T_TIMESKEW = 350, T_FULLSKEW = 351, T_PERIOD = 352, 
    T_WIDTH = 353, T_NOCHANGE = 354, T_REMOVAL = 355, T_TIMESCALE = 356, 
    T_CELLDEF = 357, T_ENDCELL = 358, T_LINE = 359, T_BEGINKEY = 360, T_ENDKEY = 361, 
    T_UNCONN = 362, T_NOUNCON = 363, T_PRAGMA = 364, T_ACCEL = 365, T_NOACC = 366, 
    T_USELIB = 367, T_DEFNET = 368, T_DEFDEC = 369, T_DEFTRI = 370, T_DELMOD = 371, 
    T_DELMODDIS = 372, T_DELMODUNIT = 373, T_DELMODZERO = 374, T_PROT = 375, 
    T_DISPORT = 376, T_ENPORT = 377, T_NOSUPP = 378, T_ENDPROT = 379, T_ENDPROTED = 380, 
    T_EXP = 381, T_NOEXP = 382, T_PROTECTE = 383, T_AUTO = 384, T_REMGATE = 385, 
    T_NONET = 386, T_RENET = 387, T_SIG = 388, T_UNSIG = 389, T_IFDEF = 390, 
    T_IFNDEF = 391, T_ENDIF = 392, T_UNDEF = 393, TT_ELSE = 394, T_ELSIF = 395, 
    T_INCLUDEDIR = 396, T_POUND = 397, Decimal_number = 398, Octal_number = 399, 
    Binary_number = 400, Hex_number = 401, Realnumber = 402, UNLIT = 403, 
    String_literal = 404, Integer = 405, Time_literal = 406, NEWLINE = 407, 
    WS = 408, Attribute_instance = 409, Block_comment = 410, Sys_macro = 411, 
    Out_line_comment = 412, One_line_comment = 413, Simple_identifier = 414, 
    System_identifier = 415, Escape_identifier = 416, Define = 417, Macro = 418, 
    MacroId = 419
  };

  explicit SysVerilogLexer(antlr4::CharStream *input);

  ~SysVerilogLexer() override;


    void doLineCount()
    { 
     /* 
      if((ipc%200)==0){
      std::cout<<"parsing line::"<<ipc<<"\n";
       }
      */
      ipc++;
    }
   


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  void action(antlr4::RuleContext *context, size_t ruleIndex, size_t actionIndex) override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
  int ipc=0; 

  // Individual action functions triggered by action() above.
  void NEWLINEAction(antlr4::RuleContext *context, size_t actionIndex);

  // Individual semantic predicate functions triggered by sempred() above.

};

