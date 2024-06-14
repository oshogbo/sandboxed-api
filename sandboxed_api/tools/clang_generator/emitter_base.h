// Copyright 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SANDBOXED_API_TOOLS_CLANG_GENERATOR_EMITTER_BASE_H_
#define SANDBOXED_API_TOOLS_CLANG_GENERATOR_EMITTER_BASE_H_

#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/container/node_hash_set.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"

namespace sapi {
// TODO b/347118045 - Refactor the naming of internal namespaces across the
// codebase.
namespace internal {

// Returns a string of the specified code reformatted to conform to the Google
// style.
// Ill-formed code will return an error status.
absl::StatusOr<std::string> ReformatGoogleStyle(const std::string& filename,
                                                const std::string& code,
                                                int column_limit = -1);

}  // namespace internal

class RenderedType {
 public:
  RenderedType(std::string ns_name, std::string spelling)
      : ns_name(std::move(ns_name)), spelling(std::move(spelling)) {}

  bool operator==(const RenderedType& other) const {
    return ns_name == other.ns_name && spelling == other.spelling;
  }

  template <typename H>
  friend H AbslHashValue(H h, RenderedType rt) {
    return H::combine(std::move(h), rt.ns_name, rt.spelling);
  }

  std::string ns_name;
  std::string spelling;
};

class EmitterBase {
 public:
  virtual ~EmitterBase() = default;

  // Adds the declarations of previously collected types to the emitter,
  // recording the spelling of each one. Types/declarations that are not
  // supported by the current generator settings or that are unwanted or
  // unnecessary are skipped. Other filtered types include C++ constructs or
  // well-known standard library elements. The latter can be replaced by
  // including the correct headers in the emitted header.
  void AddTypeDeclarations(const std::vector<clang::TypeDecl*>& type_decls);

  // Adds the declarations of previously collected functions to the emitter.
  virtual absl::Status AddFunction(clang::FunctionDecl* decl) = 0;

  // Stores namespaces and a list of spellings for types. Keeps track of types
  // that have been rendered so far. Using a node_hash_set for pointer
  // stability.
  absl::node_hash_set<RenderedType> rendered_types_;

  // A vector to preserve the order of type declarations needs to be preserved.
  std::vector<const RenderedType*> rendered_types_ordered_;

  // Fully qualified names of functions for the sandboxed API. Keeps track of
  // functions that have been rendered so far.
  absl::flat_hash_set<std::string> rendered_functions_;

 private:
  void EmitType(clang::TypeDecl* type_decl);
};

// Returns a unique name for a parameter. If `decl` has no name, a unique name
// will be generated in the form of `unnamed<index>_`.
std::string GetParamName(const clang::ParmVarDecl* decl, int index);

}  // namespace sapi

#endif  // SANDBOXED_API_TOOLS_CLANG_GENERATOR_EMITTER_BASE_H_
