//===-- sotoc/src/TargetCodeFragment.cpp ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the classes TargetCodeDecl and TargetCodeRegion.
///
//===----------------------------------------------------------------------===//

#include <sstream>

#include "clang/AST/Decl.h"
#include "clang/AST/DeclOpenMP.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtOpenMP.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Token.h"

#include "Debug.h"
#include "OmpPragma.h"
#include "TargetCodeFragment.h"

/**
 * \brief Add capture
 *
 * This will automatically create and save a \ref TargetRegionVariable
 * which holds all information to generate parameters for the generated
 * target region function.
 *
 * \param Capture Captures element
 */
void TargetCodeRegion::addCapture(const clang::CapturedStmt::Capture *Capture) {
  CapturedVars.push_back(TargetRegionVariable(Capture, CapturedLowerBounds));
}

/**
 * \brief Add OMP clause parameters
 *
 * Adds OMP clause paramenters to a TargetCodeRegion
 *
 * \param Param Parameter
 */
void TargetCodeRegion::addOMPClauseParam(clang::VarDecl *Param) {
  for (auto &CV : capturedVars()) {
    if (CV.getDecl() == Param) {
      return;
    }
  }

  if (std::find(OMPClausesParams.begin(), OMPClausesParams.end(), Param) != OMPClausesParams.end()) {
    return;
  }

  DEBUGP("Adding variable " << Param->getName() << "as OpenMP clause parameter");
  OMPClausesParams.push_back(Param);
}

/**
 * \brief Add OMP clauses
 *
 * Adds a (top level) OpenMP clause for the target region.
 * These clauses are later used to determine which OpenMP #pragma needs to be
 * generated at the top level of the target region function.
 *
 * \param Clause OMP Clause
 */
void TargetCodeRegion::addOMPClause(clang::OMPClause *Clause) {
  OMPClauses.push_back(Clause);
}

/**
 * \brief Determine whether a region has a compound statement
 *
 * \param S Statement (region)
 * \return true If the region has a compound statement
 * \return false If the region does not have a compound statement
 */
static bool hasRegionCompoundStmt(const clang::Stmt *S) {
  if (const auto *SS = llvm::dyn_cast<clang::CapturedStmt>(S)) {
    if (llvm::isa<clang::CompoundStmt>(SS->getCapturedStmt())) {
      return true;
    } else if (llvm::isa<clang::CapturedStmt>(SS->getCapturedStmt())) {
      return hasRegionCompoundStmt(SS->getCapturedStmt());
    }
  }
  return false;
}

/**
 * \brief Determine whether a region has a OMP statement
 *
 * \param S Statement (region)
 * \return true If the region has a OMP statement
 * \return false If the region does not have a OMP statement
 */
static bool hasRegionOMPStmt(const clang::Stmt *S) {
  if (const auto *SS = llvm::dyn_cast<clang::CapturedStmt>(S)) {
    if (llvm::isa<clang::OMPExecutableDirective>(SS->getCapturedStmt())) {
      return true;
    } else if (llvm::isa<clang::CapturedStmt>(SS->getCapturedStmt())) {
      return hasRegionOMPStmt(SS->getCapturedStmt());
    }
  }
  return false;
}

/**
 * \brief Get the end a OMP stmt source
 *
 * \param S Statement
 * \return clang::SourceLocation Location of the end
 */
static clang::SourceLocation getOMPStmtSourceLocEnd(const clang::Stmt *S) {
  while (auto *CS = llvm::dyn_cast<clang::CapturedStmt>(S)) {
    S = CS->getCapturedStmt();
  }

  while (auto *OmpExecS = llvm::dyn_cast<clang::OMPExecutableDirective>(S)) {
    S = OmpExecS->getInnermostCapturedStmt();
    if (auto *CS = llvm::dyn_cast<clang::CapturedStmt>(S)) {
      S = CS->getCapturedStmt();
    }
  }

  return S->getEndLoc();
}

// TODO: Implement recursive for an arbitrary depth?
/**
 * \brief Find previous token
 *
 * \param Loc Source Location
 * \param SM Source Manager
 * \param LO Language options
 * \return clang::SourceLocation Previous token
 */
static clang::SourceLocation findPreviousToken(clang::SourceLocation Loc,
                                               clang::SourceManager &SM,
                                               const clang::LangOptions &LO) {
  clang::Token token;

  Loc = clang::Lexer::GetBeginningOfToken(Loc, SM, LO);

  // Search until we find a valid token before Loc
  // TODO: Error handling if no token can be found
  do {
    Loc = clang::Lexer::GetBeginningOfToken(Loc.getLocWithOffset(-1), SM, LO);
  } while ((clang::Lexer::getRawToken(Loc, token, SM, LO)));

  return token.getLocation();
}

/**
 * \brief Destroy the Target Code Fragment:: Target Code Fragment object
 *
 */
TargetCodeFragment::~TargetCodeFragment() {}

/**
 * \brief Returns a source location at the start of a pragma in the captured statement
 *
 * \return clang::SourceLocation Start location
 */
clang::SourceLocation TargetCodeRegion::getStartLoc() {
  clang::SourceManager &SM = Context.getSourceManager();
  const clang::LangOptions &LO = Context.getLangOpts();
  auto TokenBegin = clang::Lexer::GetBeginningOfToken(
      CapturedStmtNode->getBeginLoc(), SM, LO);
  if (hasRegionCompoundStmt(CapturedStmtNode)) {

#if 0
    // This piece of code could be used to check if we start with a new scope.
    // However, the pretty printer destroys this again somehow...
    // Since the extra scope does not really hurt, i will leave it as it is for now.
    clang::Token token;
    if(!(clang::Lexer::getRawToken(TokenBegin, token, SM, LO))) {
      if (token.is(clang::tok::l_brace)) {
        auto possibleNextToken = clang::Lexer::findNextToken(
                TokenBegin, SM, LO);
        if (possibleNextToken.hasValue()) {
          return possibleNextToken.getValue().getLocation();
        } else {
          llvm::outs()<< "OUCH\n";
        }
        return TokenBegin.getLocWithOffset(1);
      }
    }
    else llvm::outs() << "NOTOK\n";
#endif

    return TokenBegin;
  } else if (hasRegionOMPStmt(CapturedStmtNode)) {
    // We have to go backwards 2 tokens in case of an OMP statement
    // (the '#' and the 'pragma').
    return findPreviousToken(findPreviousToken(TokenBegin, SM, LO), SM, LO);
  } else {
    return CapturedStmtNode->getBeginLoc();
  }
}

/**
 * \brief Get end location
 *
 * \return clang::SourceLocation End location
 */
clang::SourceLocation TargetCodeRegion::getEndLoc() {
  clang::SourceManager &SM = Context.getSourceManager();
  const clang::LangOptions &LO = Context.getLangOpts();
  auto N = CapturedStmtNode;
  if (hasRegionCompoundStmt(N)) {
    return clang::Lexer::GetBeginningOfToken(N->getEndLoc(), SM, LO)
        .getLocWithOffset(-1); // TODO: If I set this to "1" it works too. I
                               // think it was here to remove addition scope
                               // which i get with "printPretty". Does this
                               // need some fixing?
  } else if (hasRegionOMPStmt(N)) {
    return getOMPStmtSourceLocEnd(N);
  } else {
    return N->getEndLoc();
  }
}

/**
 * \brief Returns the name of the function in which the target region is declared.
 *
 * \return const std::string
 */
const std::string TargetCodeRegion::getParentFuncName() {
  return ParentFunctionDecl->getNameInfo().getAsString();
}

/**
 * \brief Get target directive location
 *
 * Returns the SourceLocation for the target directive (we need the source
 * location of the first pragma of the target region to compose the name of
 * the function generated for that region)
 *
 * \return clang::SourceLocation Location
 */
clang::SourceLocation TargetCodeRegion::getTargetDirectiveLocation() {
  return TargetDirective->getBeginLoc();
}

/**
 * \brief Get source range
 *
 * \return clang::SourceRange
 */
clang::SourceRange TargetCodeRegion::getRealRange() {
  return CapturedStmtNode->getSourceRange();
}

/**
 * \brief Get spelling range
 *
 * \return clang::SourceRange
 */
clang::SourceRange TargetCodeRegion::getSpellingRange() {
  auto &SM =
      CapturedStmtNode->getCapturedDecl()->getASTContext().getSourceManager();
  auto InnerRange = getInnerRange();
  return clang::SourceRange(SM.getSpellingLoc(InnerRange.getBegin()),
                            SM.getSpellingLoc(InnerRange.getEnd()));
}

/**
 * \brief Get inner range
 *
 * \return clang::SourceRange
 */
clang::SourceRange TargetCodeRegion::getInnerRange() {
  auto InnerLocStart = getStartLoc();
  auto InnerLocEnd = getEndLoc();
  return clang::SourceRange(InnerLocStart, InnerLocEnd);
}

/**
 * \brief Print Helper Class
 *
 */
class TargetRegionPrinterHelper : public clang::PrinterHelper {
  clang::PrintingPolicy PP;

public:
  /**
   * \brief Construct a new Target Region Printer Helper object
   *
   * \param PP
   */
  TargetRegionPrinterHelper(clang::PrintingPolicy PP) : PP(PP){};
  /**
   * \brief Handle Statement
   *
   * \param E Statement
   * \param OS Out stream
   */
  bool handledStmt(clang::Stmt *E, llvm::raw_ostream &OS) {
    if (auto *Directive = llvm::dyn_cast<clang::OMPExecutableDirective>(E)) {
      if (OmpPragma::isReplaceable(Directive)) {
        OmpPragma(Directive, PP).printReplacement(OS);
        OS << "\n";
        Directive->child_begin()->printPretty(OS, this, PP);
        return true;
      }

      if (OmpPragma::needsAdditionalPragma(Directive)) {
        OmpPragma(Directive, PP).printAddition(OS);
        OS << "\n";
        return false;
      }
    }
    return false;
  }
};

/**
 * \brief Do pretty printing in order to resolve Macros
 *
 * \return std::string Pretty output
 */
std::string TargetCodeRegion::PrintPretty() {
  // TODO: Is there a better approach (e.g., token or preprocessor based?)
  // One issue here: Addition braces (i.e., scope) in some cases.
  std::string PrettyStr = "";
  llvm::raw_string_ostream PrettyOS(PrettyStr);
  TargetRegionPrinterHelper Helper(PP);
  if (CapturedStmtNode != NULL)
    CapturedStmtNode->printPretty(PrettyOS, &Helper, PP);
  return PrettyOS.str();
}

/**
 * \brief Get source range
 *
 * \return clang::SourceRange
 */
clang::SourceRange TargetCodeDecl::getRealRange() {
  return DeclNode->getSourceRange();
}

/**
 * \brief Get spelling range
 *
 * \return clang::SourceRange
 */
clang::SourceRange TargetCodeDecl::getSpellingRange() {
  auto &SM = DeclNode->getASTContext().getSourceManager();
  auto InnerRange = getInnerRange();
  return clang::SourceRange(SM.getSpellingLoc(InnerRange.getBegin()),
                            SM.getSpellingLoc(InnerRange.getEnd()));
}

/**
 * \brief Do pretty printing
 *
 * \return std::string Pretty output
 */
std::string TargetCodeDecl::PrintPretty() {
  std::string PrettyStr = "";
  llvm::raw_string_ostream PrettyOS(PrettyStr);

  // This hack solves our problem with structs and enums being autoexpanded#
  // sometimes (See comment in Issue #20.
  clang::PrintingPolicy LocalPP(PP);
  if (llvm::isa<clang::TypedefDecl>(DeclNode)) {
    LocalPP.IncludeTagDefinition = 1;
  }

  TargetRegionPrinterHelper Helper(PP);

  // This hack removes the 'static' keyword from globalVarDecls, because we
  // cannot find variables from the host if they are static.
  bool HasStaticKeyword = false;
  if (auto *VarDeclNode = llvm::dyn_cast<clang::VarDecl>(DeclNode)) {
    if (VarDeclNode->getStorageClass() == clang::SC_Static) {
      HasStaticKeyword = true;
      VarDeclNode->setStorageClass(clang::SC_None);
    }
  }

  DeclNode->print(PrettyOS, LocalPP, 0, false, &Helper);

  // Add static storage class back so (hopefully) this doesnt break anyting
  // (but it totally will).
  if (auto *VarDeclNode = llvm::dyn_cast<clang::VarDecl>(DeclNode)) {
    if (HasStaticKeyword) {
      VarDeclNode->setStorageClass(clang::SC_Static);
    }
  }

  // This hack removes '#pragma omp declare target' from the output
  std::string outString = PrettyOS.str();
  const char *declareTargetPragma = "#pragma omp declare target";

  if (outString.compare(0, strlen(declareTargetPragma), declareTargetPragma) ==
      0) {
    outString = outString.substr(strlen(declareTargetPragma));
  }
  return outString;
}
