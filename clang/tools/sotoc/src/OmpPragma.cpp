//===-- sotoc/src/TargetCode ------------------------ ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the class OmpPragma, which is used to generate repla-
/// cement pragmas for teams and team combined constructs.
///
//===----------------------------------------------------------------------===//

#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/OpenMPKinds.h"
#include <ctype.h>

#include "OmpPragma.h"

int ClauseParamCounter = -1;

/**
 * \brief Print replacement pragmas
 *
 * In some cases we have to modify the printed pragma.
 * If we have a combined construct using `target`, remove `target` because we
 * are already running on the target device.
 * If we have a combined construct using `teams`, remove `teams` because the
 * runtime can decide to spawn only a single team.
 * If we have a combined construct using `distribute`, remove `distribute`
 * because it can only be part of a teams directive and we only use one team.
 * If we have a `simd` (either alone or in a combined construct), prepend
 * `#pragma _NEC ivdep` to indicate to `ncc` that there are no dependencies.
 *
 * \param Out Output stream
 */
void OmpPragma::printReplacement(llvm::raw_ostream &Out) {
  switch (Kind) {
  case clang::OpenMPDirectiveKind::OMPD_target_parallel: {
    Out << "  #pragma omp parallel ";
    break;
  }
  case clang::OpenMPDirectiveKind::OMPD_target_parallel_for:
  case clang::OpenMPDirectiveKind::OMPD_distribute_parallel_for:
  case clang::OpenMPDirectiveKind::OMPD_teams_distribute_parallel_for:
  case clang::OpenMPDirectiveKind::OMPD_target_teams_distribute_parallel_for: {
    Out << "  #pragma omp parallel for ";
    break;
  }
  case clang::OpenMPDirectiveKind::OMPD_target_parallel_for_simd:
  case clang::OpenMPDirectiveKind::OMPD_distribute_parallel_for_simd:
  case clang::OpenMPDirectiveKind::OMPD_teams_distribute_parallel_for_simd:
  case clang::OpenMPDirectiveKind::
        OMPD_target_teams_distribute_parallel_for_simd: {
    Out << "  #pragma _NEC ivdep\n  #pragma omp parallel for simd ";
    break;
  }
  case clang::OpenMPDirectiveKind::OMPD_target_simd:
  case clang::OpenMPDirectiveKind::OMPD_distribute_simd:
  case clang::OpenMPDirectiveKind::OMPD_teams_distribute_simd:
  case clang::OpenMPDirectiveKind::OMPD_target_teams_distribute_simd: {
    Out << "  #pragma _NEC ivdep\n  #pragma omp simd ";
    break;
  }
  default:
    return;
  }
  printClauses(Out);
}

//TODO: Do we need this?
void OmpPragma::printAddition(llvm::raw_ostream &Out) {
  Out << "  #pragma _NEC ivdep ";
}

/**
 * \brief Determines whether a pragma is replacable
 *
 * \param Directive Given Directive
 * \return true If the directive is replacable
 * \return false If the directive is not replacable
 */
bool OmpPragma::isReplaceable(clang::OMPExecutableDirective *Directive) {
  return (clang::isOpenMPTeamsDirective(Directive->getDirectiveKind()) ||
      clang::isOpenMPDistributeDirective(Directive->getDirectiveKind()));
}

/**
 * \brief Determines whether a additional pragma is needed
 *
 * \param Directive given directive
 * \return true Directive needs an additional pragma
 * \return false Directive does not need an additional pragma
 */
bool OmpPragma::needsAdditionalPragma(
    clang::OMPExecutableDirective *Directive) {
  if (llvm::isa<clang::OMPForSimdDirective>(Directive) ||
      llvm::isa<clang::OMPParallelForSimdDirective>(Directive) ||
      llvm::isa<clang::OMPSimdDirective>(Directive) ||
      llvm::isa<clang::OMPTaskLoopSimdDirective>(Directive)) {
    return true;
  }
  return false;
}

/**
 * \brief Determine whether a clause is printable
 *
 * Checks for a clause the clause kind and determines which clauses are printable.
 *
 * \param Clause Clause to check
 * \return true If the clause is printable
 * \return false If the clause is not printable
 */
bool OmpPragma::isClausePrintable(clang::OMPClause *Clause) {
  switch (Kind) {
  case clang::OpenMPDirectiveKind::OMPD_target: {
    switch (Clause->getClauseKind()) {
    // case clang::OpenMPClauseKind::OMPC_if:
    // case clang::OpenMPClauseKind::OMPC_device:
    // case clang::OpenMPClauseKind::OMPC_map:
    case clang::OpenMPClauseKind::OMPC_private:
    // case clang::OpenMPClauseKind::OMPC_nowait:
    // case clang::OpenMPClauseKind::OMPC_depend:
    // case clang::OpenMPClauseKind::OMPC_defaultmap:
    case clang::OpenMPClauseKind::OMPC_firstprivate:
      // case clang::OpenMPClauseKind::OMPC_is_device_ptr:
      // case clang::OpenMPClauseKind::OMPC_reduction:
      return true;
    default:
      return false;
    }
  }
  /*case clang::OpenMPDirectiveKind::OMPD_target_teams: {
    switch (Clause->getClauseKind()) {
    // case clang::OpenMPClauseKind::OMPC_map:
    case clang::OpenMPClauseKind::OMPC_default:
    case clang::OpenMPClauseKind::OMPC_private:
    case clang::OpenMPClauseKind::OMPC_firstprivate:
    case clang::OpenMPClauseKind::OMPC_shared:
    case clang::OpenMPClauseKind::OMPC_reduction:
    case clang::OpenMPClauseKind::OMPC_num_teams:
    case clang::OpenMPClauseKind::OMPC_thread_limit:
      return true;
    default:
      return false;
    }
  }*/
  case clang::OpenMPDirectiveKind::OMPD_target_parallel: {
    switch (Clause->getClauseKind()) {
    // case clang::OpenMPClauseKind::OMPC_map:
    case clang::OpenMPClauseKind::OMPC_num_threads:
    case clang::OpenMPClauseKind::OMPC_default:
    case clang::OpenMPClauseKind::OMPC_proc_bind:
    case clang::OpenMPClauseKind::OMPC_private:
    case clang::OpenMPClauseKind::OMPC_firstprivate:
    case clang::OpenMPClauseKind::OMPC_shared:
      // case clang::OpenMPClauseKind::OMPC_reduction:
      return true;
    default:
      return false;
    case clang::OpenMPClauseKind::OMPC_if:
      clang::OMPIfClause *IC =
          llvm::dyn_cast_or_null<clang::OMPIfClause>(Clause);
      if ((IC->getNameModifier()) == clang::OpenMPDirectiveKind::OMPD_target) {
        return false;
      } else {
        return true;
      }
    }
  }
  case clang::OpenMPDirectiveKind::OMPD_target_parallel_for: {
    switch (Clause->getClauseKind()) {
    // case clang::OpenMPClauseKind::OMPC_map:
    case clang::OpenMPClauseKind::OMPC_num_threads:
    case clang::OpenMPClauseKind::OMPC_default:
    case clang::OpenMPClauseKind::OMPC_proc_bind:
    case clang::OpenMPClauseKind::OMPC_private:
    case clang::OpenMPClauseKind::OMPC_firstprivate:
    case clang::OpenMPClauseKind::OMPC_shared:
    case clang::OpenMPClauseKind::OMPC_reduction:
    case clang::OpenMPClauseKind::OMPC_lastprivate:
    case clang::OpenMPClauseKind::OMPC_collapse:
    case clang::OpenMPClauseKind::OMPC_schedule:
    case clang::OpenMPClauseKind::OMPC_ordered:
    case clang::OpenMPClauseKind::OMPC_linear:
      return true;
    default:
      return false;
    case clang::OpenMPClauseKind::OMPC_if:
      clang::OMPIfClause *IC =
          llvm::dyn_cast_or_null<clang::OMPIfClause>(Clause);
      if ((IC->getNameModifier()) == clang::OpenMPDirectiveKind::OMPD_target) {
        return false;
      } else {
        return true;
      }
    }
  }
  case clang::OpenMPDirectiveKind::OMPD_target_parallel_for_simd: {
    switch (Clause->getClauseKind()) {
    // case clang::OpenMPClauseKind::OMPC_map:
    case clang::OpenMPClauseKind::OMPC_num_threads:
    case clang::OpenMPClauseKind::OMPC_default:
    case clang::OpenMPClauseKind::OMPC_proc_bind:
    case clang::OpenMPClauseKind::OMPC_private:
    case clang::OpenMPClauseKind::OMPC_firstprivate:
    case clang::OpenMPClauseKind::OMPC_shared:
    case clang::OpenMPClauseKind::OMPC_reduction:
    case clang::OpenMPClauseKind::OMPC_lastprivate:
    case clang::OpenMPClauseKind::OMPC_collapse:
    case clang::OpenMPClauseKind::OMPC_schedule:
    case clang::OpenMPClauseKind::OMPC_safelen:
    case clang::OpenMPClauseKind::OMPC_simdlen:
    case clang::OpenMPClauseKind::OMPC_linear:
    case clang::OpenMPClauseKind::OMPC_aligned:
    case clang::OpenMPClauseKind::OMPC_ordered:
      return true;
    default:
      return false;
    case clang::OpenMPClauseKind::OMPC_if:
      clang::OMPIfClause *IC =
          llvm::dyn_cast_or_null<clang::OMPIfClause>(Clause);
      if ((IC->getNameModifier()) == clang::OpenMPDirectiveKind::OMPD_target) {
        return false;
      } else {
        return true;
      }
    }
  }
  case clang::OpenMPDirectiveKind::OMPD_target_simd: {
    switch (Clause->getClauseKind()) {
    // case clang::OpenMPClauseKind::OMPC_map:
    case clang::OpenMPClauseKind::OMPC_private:
    case clang::OpenMPClauseKind::OMPC_lastprivate:
    case clang::OpenMPClauseKind::OMPC_linear:
    case clang::OpenMPClauseKind::OMPC_aligned:
    case clang::OpenMPClauseKind::OMPC_safelen:
    case clang::OpenMPClauseKind::OMPC_simdlen:
    case clang::OpenMPClauseKind::OMPC_collapse:
    case clang::OpenMPClauseKind::OMPC_reduction:
      return true;
    default:
      return false;
    }
  }
  case clang::OpenMPDirectiveKind::OMPD_target_teams_distribute: {
    switch (Clause->getClauseKind()) {
    // case clang::OpenMPClauseKind::OMPC_map:
    case clang::OpenMPClauseKind::OMPC_default:
    case clang::OpenMPClauseKind::OMPC_private:
    case clang::OpenMPClauseKind::OMPC_firstprivate:
    case clang::OpenMPClauseKind::OMPC_shared:
    case clang::OpenMPClauseKind::OMPC_reduction:
    case clang::OpenMPClauseKind::OMPC_thread_limit:
    case clang::OpenMPClauseKind::OMPC_lastprivate:
    case clang::OpenMPClauseKind::OMPC_collapse:
    case clang::OpenMPClauseKind::OMPC_dist_schedule:
      return true;
    default:
      return false;
    }
  }
  case clang::OpenMPDirectiveKind::OMPD_teams_distribute_parallel_for:
  case clang::OpenMPDirectiveKind::OMPD_target_teams_distribute_parallel_for: {
    switch (Clause->getClauseKind()) {
    // case clang::OpenMPClauseKind::OMPC_map:
    case clang::OpenMPClauseKind::OMPC_firstprivate:
    case clang::OpenMPClauseKind::OMPC_lastprivate:
    case clang::OpenMPClauseKind::OMPC_collapse:
    case clang::OpenMPClauseKind::OMPC_dist_schedule:
    case clang::OpenMPClauseKind::OMPC_num_threads:
    case clang::OpenMPClauseKind::OMPC_default:
    case clang::OpenMPClauseKind::OMPC_proc_bind:
    case clang::OpenMPClauseKind::OMPC_private:
    case clang::OpenMPClauseKind::OMPC_shared:
    case clang::OpenMPClauseKind::OMPC_reduction:
    case clang::OpenMPClauseKind::OMPC_schedule:
    case clang::OpenMPClauseKind::OMPC_thread_limit:
      return true;
    default:
      return false;
    case clang::OpenMPClauseKind::OMPC_if:
      clang::OMPIfClause *IC =
          llvm::dyn_cast_or_null<clang::OMPIfClause>(Clause);
      if ((IC->getNameModifier()) == clang::OpenMPDirectiveKind::OMPD_target) {
        return false;
      } else {
        return true;
      }
    }
  }
  case clang::OpenMPDirectiveKind::OMPD_teams_distribute_parallel_for_simd:
  case clang::OpenMPDirectiveKind::
      OMPD_target_teams_distribute_parallel_for_simd: {
    switch (Clause->getClauseKind()) {
    // case clang::OpenMPClauseKind::OMPC_map:
    case clang::OpenMPClauseKind::OMPC_firstprivate:
    case clang::OpenMPClauseKind::OMPC_lastprivate:
    case clang::OpenMPClauseKind::OMPC_collapse:
    case clang::OpenMPClauseKind::OMPC_dist_schedule:
    case clang::OpenMPClauseKind::OMPC_num_threads:
    case clang::OpenMPClauseKind::OMPC_default:
    case clang::OpenMPClauseKind::OMPC_proc_bind:
    case clang::OpenMPClauseKind::OMPC_private:
    case clang::OpenMPClauseKind::OMPC_shared:
    case clang::OpenMPClauseKind::OMPC_reduction:
    case clang::OpenMPClauseKind::OMPC_schedule:
    case clang::OpenMPClauseKind::OMPC_linear:
    case clang::OpenMPClauseKind::OMPC_aligned:
    case clang::OpenMPClauseKind::OMPC_safelen:
    case clang::OpenMPClauseKind::OMPC_simdlen:
    case clang::OpenMPClauseKind::OMPC_thread_limit:
      return true;
    default:
      return false;
    case clang::OpenMPClauseKind::OMPC_if:
      clang::OMPIfClause *IC =
          llvm::dyn_cast_or_null<clang::OMPIfClause>(Clause);
      if ((IC->getNameModifier()) == clang::OpenMPDirectiveKind::OMPD_target) {
        return false;
      } else {
        return true;
      }
    }
  }
  case clang::OpenMPDirectiveKind::OMPD_teams_distribute_simd:
  case clang::OpenMPDirectiveKind::OMPD_target_teams_distribute_simd: {
    switch (Clause->getClauseKind()) {
    // case clang::OpenMPClauseKind::OMPC_map:
    case clang::OpenMPClauseKind::OMPC_default:
    case clang::OpenMPClauseKind::OMPC_private:
    case clang::OpenMPClauseKind::OMPC_firstprivate:
    case clang::OpenMPClauseKind::OMPC_shared:
    case clang::OpenMPClauseKind::OMPC_reduction:
    case clang::OpenMPClauseKind::OMPC_thread_limit:
    case clang::OpenMPClauseKind::OMPC_lastprivate:
    case clang::OpenMPClauseKind::OMPC_collapse:
    case clang::OpenMPClauseKind::OMPC_dist_schedule:
    case clang::OpenMPClauseKind::OMPC_linear:
    case clang::OpenMPClauseKind::OMPC_aligned:
    case clang::OpenMPClauseKind::OMPC_safelen:
    case clang::OpenMPClauseKind::OMPC_simdlen:
      return true;
    default:
      return false;
    case clang::OpenMPClauseKind::OMPC_if:
      clang::OMPIfClause *IC =
          llvm::dyn_cast_or_null<clang::OMPIfClause>(Clause);
      if ((IC->getNameModifier()) == clang::OpenMPDirectiveKind::OMPD_target) {
        return false;
      } else {
        return true;
      }
    }
  }
  default:
    break;
  }
  return false;
}

/**
 * \brief Print OMP Clauses
 *
 * \param Out Out stream
 */
void OmpPragma::printClauses(llvm::raw_ostream &Out) {
  std::string InString;
  llvm::raw_string_ostream In(InString);
  clang::OMPClausePrinter Printer(In, PP);

  bool numThreads = 0;
  std::string numThreadsParam;
  bool threadLimit = 0;
  std::string threadLimitParam;

  for (auto C : Clauses) {
    // Only print clauses that are both printable (for us) and are actually in
    // the users code (is explicit)
    if (isClausePrintable(C) && !C->isImplicit()) {
      Printer.Visit(C);

      In.str();
      size_t inp = InString.find("(") + 1;
      size_t paramlength = InString.length() - inp - 1;
      std::string param = InString.substr(inp, paramlength);
      InString.erase(inp, paramlength);

      if (C->getClauseKind() == clang::OpenMPClauseKind::OMPC_num_threads) {
        rewriteParam(&param);
        numThreadsParam = param;
        numThreads = true;
      } else if (C->getClauseKind() ==
                 clang::OpenMPClauseKind::OMPC_thread_limit) {
        rewriteParam(&param);
        threadLimitParam = param;
        threadLimit = true;
      } else {
        Out << InString.insert(inp, param) << " ";
      }

      InString.clear();
    }
  }

  if (numThreads && threadLimit) {
    Out << "num_threads((" << numThreadsParam << " < " << threadLimitParam
        << ") ? " << numThreadsParam << " : " << threadLimitParam << ") ";
  } else if (numThreads && !threadLimit) {
    Out << "num_threads(" << numThreadsParam << ") ";
  } else if (!numThreads && threadLimit) {
    Out << "num_threads(" << threadLimitParam << ") ";
  }
}

/**
 * \brief Rewrite clause parameters
 *
 * Rewrites OMP clause parameters if they are variables to replace the variable name
 * with the one we will use as the function argument.
 *
 * \param In Parameter as string (everything in brackets)
 */
void OmpPragma::rewriteParam(std::string *In) {
  bool isNumerical = true;

  for (auto i : *In) {
    if (!isdigit(i)) {
      isNumerical = false;
    }
  }

  if (!isNumerical) {
    *In = "__sotoc_clause_param_" + std::to_string(ClauseParamCounter);
    ClauseParamCounter++;
  }
}

/**
 * \brief Determines whether a structured block is needed for a pragma
 *
 * \return true If a structured block is needed
 * \return false If no structured block is needed
 */
bool OmpPragma::needsStructuredBlock() {
  switch (Kind) {
  case clang::OpenMPDirectiveKind::OMPD_target_parallel:
  case clang::OpenMPDirectiveKind::OMPD_target_simd:
  case clang::OpenMPDirectiveKind::OMPD_target_teams_distribute_simd:
    return true;
  default:
    return false;
  }
}
