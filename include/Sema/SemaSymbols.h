//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaSymbols.h - SemaSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_SYMBOLS_H
#define FLY_SEMA_SYMBOLS_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

#include <map>

namespace fly {

    class Sema;
    class ASTModule;
    class ASTGlobalVar;
    class ASTFunction;
    class ASTIdentity;
    class ASTClassAttribute;
    class ASTClassMethod;

    class SemaSymbols {

        friend class Sema;
        friend class SemaResolver;

        Sema &S;

        llvm::StringRef NameSpace;

        llvm::SmallVector<ASTModule *, 8> Modules;

        // Global Vars
        llvm::StringMap<ASTGlobalVar *> GlobalVars;

        // Functions
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> Functions;

        // Identities: Class, Enum
        llvm::StringMap<ASTIdentity *> Identities;

        // Class Attributes
        llvm::StringMap<llvm::StringMap<ASTClassAttribute *>> ClassAttributes;

        // Class Functions
        llvm::StringMap<llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassMethod *, 4>>>> ClassMethods;

        explicit SemaSymbols(Sema &S);

    public:

        const llvm::StringMap<ASTGlobalVar *> &getGlobalVars() const;

        const llvm::StringMap<std::map<uint64_t, llvm::SmallVector<ASTFunction *, 4>>> &getFunctions() const;

        const llvm::StringMap<ASTIdentity *> &getIdentities() const;

        template<class T>
        bool ContainsFunction(llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> &Functions,
                              T *NewFunction) {
            // Search by Name
            const auto &StrMapIt = Functions.find(NewFunction->getName());
            if (StrMapIt != Functions.end()) {

                // Search by Number of Parameters
                const auto IntMapIt = StrMapIt->second.find(NewFunction->getParams().size());

                // Search by Type of Parameters
                llvm::SmallVector<T *, 4> VectorFunctions = IntMapIt->second;
                for (auto &Function: VectorFunctions) {

                    // Check if NewFunction have no params
                    if (NewFunction->getParams().empty() && Function->getParams().empty()) {
                        return true;
                    }

                    // Check types
                    if (!S.Validator->CheckParams(Function->getParams(), NewFunction->getParams())) {
                        return true;
                    }
                }
            }
            return false;
        }

        template<class T>
        static bool InsertFunction(llvm::StringMap<std::map<uint64_t, llvm::SmallVector<T *, 4>>> &Functions,
                            T *NewFunction) {

            // Functions is llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTFunction *, 4>>>
            const auto &StrMapIt = Functions.find(NewFunction->getName());
            if (StrMapIt == Functions.end()) { // This Module not contains a Function with this Function->Name

                // add to llvm::SmallVector
                llvm::SmallVector<T *, 4> Vect;
                Vect.push_back(NewFunction);

                // add to std::map
                std::map<uint64_t, llvm::SmallVector<T *, 4>> IntMap;
                IntMap.insert(std::make_pair(NewFunction->getParams().size(), Vect));

                // add to llvm::StringMap
                return Functions.insert(std::make_pair(NewFunction->getName(), IntMap)).second;
            } else {
                return InsertFunction(StrMapIt->second, NewFunction);
            }
        }

        template<class T>
        static bool InsertFunction(std::map<uint64_t, llvm::SmallVector<T *, 4>> &Functions, T *NewFunction) {

            // This Module contains a Function with this Function->Name
            const auto &IntMapIt = Functions.find(NewFunction->getParams().size());
            if (IntMapIt == Functions.end()) { // but not have the same number of Params

                // add to llvm::SmallVector
                llvm::SmallVector<T *, 4> VectorFunctions;
                VectorFunctions.push_back(NewFunction);

                // add to std::map
                std::pair<uint64_t, llvm::SmallVector<T *, 4>> IntMapPair = std::make_pair(
                        NewFunction->getParams().size(), VectorFunctions);

                return Functions.insert(std::make_pair(NewFunction->getParams().size(), VectorFunctions)).second;
            } else { // This Module contains a Function with this Function->Name and same number of Params
                llvm::SmallVector<T *, 4> VectorFunctions = IntMapIt->second;
                for (auto &Function: VectorFunctions) {

                    // check no params duplicates
                    if (NewFunction->getParams().empty() && Function->getParams().empty()) {
                        // Error:
                        return false;
                    }

//                    if (!SemaValidator::CheckParams(Function->getParams(), NewFunction->getParams())) {
//                        return false;
//                    }
                }

                // Add to function list
                IntMapIt->second.push_back(NewFunction);
                return true;
            }
        }

    };

}  // end namespace fly

#endif