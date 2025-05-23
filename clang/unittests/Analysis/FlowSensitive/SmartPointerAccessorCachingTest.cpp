//===- unittests/Analysis/FlowSensitive/SmartPointerAccessorCachingTest.cpp ==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/Analysis/FlowSensitive/SmartPointerAccessorCaching.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Testing/TestAST.h"
#include "llvm/ADT/StringRef.h"
#include "gtest/gtest.h"

namespace clang::dataflow {
namespace {

using clang::ast_matchers::match;

template <typename MatcherT>
bool matches(llvm::StringRef Decls, llvm::StringRef TestInput,
             MatcherT Matcher) {
  TestAST InputAST(Decls.str() + TestInput.str());
  return !match(Matcher, InputAST.context()).empty();
}

TEST(SmartPointerAccessorCachingTest, MatchesClassWithStarArrowGet) {
  llvm::StringRef Decls(R"cc(
    namespace std {
    template <class T>
    struct unique_ptr {
      T* operator->() const;
      T& operator*() const;
      T* get() const;
    };
    }  // namespace std

    template <class T>
    using UniquePtrAlias = std::unique_ptr<T>;

    struct S { int i; };
  )cc");

  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S> P) { return (*P).i; }",
                      isSmartPointerLikeOperatorStar()));
  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S> P) { return (*P).i; }",
                      isPointerLikeOperatorStar()));

  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S> P) { return P->i; }",
                      isSmartPointerLikeOperatorArrow()));
  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S> P) { return P->i; }",
                      isPointerLikeOperatorArrow()));

  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S> P) { return P.get()->i; }",
                      isSmartPointerLikeGetMethodCall()));

  EXPECT_TRUE(matches(Decls, "int target(UniquePtrAlias<S> P) { return P->i; }",
                      isSmartPointerLikeOperatorArrow()));
  EXPECT_TRUE(matches(Decls, "int target(UniquePtrAlias<S> P) { return P->i; }",
                      isPointerLikeOperatorArrow()));
}

TEST(SmartPointerAccessorCachingTest, MatchesClassWithStarArrow) {
  llvm::StringRef Decls(R"cc(
    namespace std {
    template <class T>
    struct unique_ptr {
      T* operator->() const;
      T& operator*() const;
    };
    }  // namespace std

    template <class T>
    using UniquePtrAlias = std::unique_ptr<T>;

    struct S { int i; };
  )cc");

  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S> P) { return (*P).i; }",
                       isSmartPointerLikeOperatorStar()));
  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S> P) { return (*P).i; }",
                      isPointerLikeOperatorStar()));

  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S> P) { return P->i; }",
                       isSmartPointerLikeOperatorArrow()));
  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S> P) { return P->i; }",
                      isPointerLikeOperatorArrow()));

  EXPECT_FALSE(matches(Decls,
                       "int target(UniquePtrAlias<S> P) { return P->i; }",
                       isSmartPointerLikeOperatorArrow()));
  EXPECT_TRUE(matches(Decls, "int target(UniquePtrAlias<S> P) { return P->i; }",
                      isPointerLikeOperatorArrow()));
}

TEST(SmartPointerAccessorCachingTest, NoMatchIfUnexpectedReturnTypes) {
  llvm::StringRef Decls(R"cc(
    namespace std {
    // unique_ptr isn't really like this, but we aren't matching by name
    template <class T, class U>
    struct unique_ptr {
      U* operator->() const;
      T& operator*() const;
      T* get() const;
    };
    }  // namespace std

    struct S { int i; };
    struct T { int j; };
  )cc");

  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S, T> P) { return (*P).i; }",
                       isSmartPointerLikeOperatorStar()));
  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S, T> P) { return (*P).i; }",
                       isPointerLikeOperatorStar()));

  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S, T> P) { return P->j; }",
                       isSmartPointerLikeOperatorArrow()));
  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S, T> P) { return P->j; }",
                       isPointerLikeOperatorArrow()));
  // The class matching arguably accidentally matches, just because the
  // instantiation is with S, S. Hopefully doesn't happen too much in real code
  // with such operator* and operator-> overloads.
  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S, S> P) { return P->i; }",
                      isSmartPointerLikeOperatorArrow()));
  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S, S> P) { return P->i; }",
                      isPointerLikeOperatorArrow()));
}

TEST(SmartPointerAccessorCachingTest, NoMatchIfBinaryStar) {
  llvm::StringRef Decls(R"cc(
    namespace std {
    template <class T>
    struct unique_ptr {
      T* operator->() const;
      T& operator*(int x) const;
      T* get() const;
    };
    }  // namespace std

    struct S { int i; };
  )cc");

  EXPECT_FALSE(
      matches(Decls, "int target(std::unique_ptr<S> P) { return (P * 10).i; }",
              isSmartPointerLikeOperatorStar()));
  EXPECT_FALSE(
      matches(Decls, "int target(std::unique_ptr<S> P) { return (P * 10).i; }",
              isPointerLikeOperatorStar()));
}

TEST(SmartPointerAccessorCachingTest, NoMatchIfNoConstOverloads) {
  llvm::StringRef Decls(R"cc(
    namespace std {
    template <class T>
    struct unique_ptr {
      T* operator->();
      T& operator*();
      T* get();
    };
    }  // namespace std

    struct S { int i; };
  )cc");

  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S> P) { return (*P).i; }",
                       isSmartPointerLikeOperatorStar()));
  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S> P) { return (*P).i; }",
                       isPointerLikeOperatorStar()));

  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S> P) { return P->i; }",
                       isSmartPointerLikeOperatorArrow()));
  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S> P) { return P->i; }",
                       isPointerLikeOperatorArrow()));

  EXPECT_FALSE(
      matches(Decls, "int target(std::unique_ptr<S> P) { return P.get()->i; }",
              isSmartPointerLikeGetMethodCall()));
}

TEST(SmartPointerAccessorCachingTest, NoMatchIfNoStarMethod) {
  llvm::StringRef Decls(R"cc(
    namespace std {
    template <class T>
    struct unique_ptr {
      T* operator->();
      T* get();
    };
    }  // namespace std

    struct S { int i; };
  )cc");

  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S> P) { return P->i; }",
                       isSmartPointerLikeOperatorArrow()));
  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S> P) { return P->i; }",
                       isPointerLikeOperatorArrow()));

  EXPECT_FALSE(matches(Decls,
                       "int target(std::unique_ptr<S> P) { return P->i; }",
                       isSmartPointerLikeGetMethodCall()));
}

TEST(SmartPointerAccessorCachingTest, MatchesWithValueAndNonConstOverloads) {
  llvm::StringRef Decls(R"cc(
    namespace std {
    template <class T>
    struct optional {
      const T* operator->() const;
      T* operator->();
      const T& operator*() const;
      T& operator*();
      const T& value() const;
      T& value();
    };
    }  // namespace std

    struct S { int i; };
  )cc");

  EXPECT_TRUE(matches(
      Decls, "int target(std::optional<S> &NonConst) { return (*NonConst).i; }",
      isSmartPointerLikeOperatorStar()));
  EXPECT_TRUE(matches(
      Decls, "int target(std::optional<S> &NonConst) { return (*NonConst).i; }",
      isPointerLikeOperatorStar()));

  EXPECT_TRUE(matches(
      Decls, "int target(const std::optional<S> &Const) { return (*Const).i; }",
      isSmartPointerLikeOperatorStar()));
  EXPECT_TRUE(matches(
      Decls, "int target(const std::optional<S> &Const) { return (*Const).i; }",
      isPointerLikeOperatorStar()));

  EXPECT_TRUE(matches(
      Decls, "int target(std::optional<S> &NonConst) { return NonConst->i; }",
      isSmartPointerLikeOperatorArrow()));
  EXPECT_TRUE(matches(
      Decls, "int target(std::optional<S> &NonConst) { return NonConst->i; }",
      isPointerLikeOperatorArrow()));

  EXPECT_TRUE(matches(
      Decls, "int target(const std::optional<S> &Const) { return Const->i; }",
      isSmartPointerLikeOperatorArrow()));
  EXPECT_TRUE(matches(
      Decls, "int target(const std::optional<S> &Const) { return Const->i; }",
      isPointerLikeOperatorArrow()));

  EXPECT_TRUE(matches(
      Decls,
      "int target(std::optional<S> &NonConst) { return NonConst.value().i; }",
      isSmartPointerLikeValueMethodCall()));
  EXPECT_TRUE(matches(
      Decls,
      "int target(const std::optional<S> &Const) { return Const.value().i; }",
      isSmartPointerLikeValueMethodCall()));
}

TEST(SmartPointerAccessorCachingTest, MatchesWithTypeAliases) {
  llvm::StringRef Decls(R"cc(
    template <class T>
    struct HasGetAndValue {
      using pointer_t = T*;
      using reference_t = T&;

      const pointer_t operator->() const;
      pointer_t operator->();
      const reference_t operator*() const;
      reference_t operator*();
      const reference_t value() const;
      reference_t value();
      const pointer_t get() const;
      pointer_t get();
    };

    struct S { int i; };
  )cc");

  EXPECT_TRUE(matches(
      Decls,
      "int target(HasGetAndValue<S> &NonConst) { return (*NonConst).i; }",
      isSmartPointerLikeOperatorStar()));
  EXPECT_TRUE(matches(
      Decls,
      "int target(HasGetAndValue<S> &NonConst) { return (*NonConst).i; }",
      isPointerLikeOperatorStar()));

  EXPECT_TRUE(matches(
      Decls,
      "int target(const HasGetAndValue<S> &Const) { return (*Const).i; }",
      isSmartPointerLikeOperatorStar()));
  EXPECT_TRUE(matches(
      Decls,
      "int target(const HasGetAndValue<S> &Const) { return (*Const).i; }",
      isPointerLikeOperatorStar()));

  EXPECT_TRUE(matches(
      Decls, "int target(HasGetAndValue<S> &NonConst) { return NonConst->i; }",
      isSmartPointerLikeOperatorArrow()));
  EXPECT_TRUE(matches(
      Decls, "int target(HasGetAndValue<S> &NonConst) { return NonConst->i; }",
      isPointerLikeOperatorArrow()));

  EXPECT_TRUE(matches(
      Decls, "int target(const HasGetAndValue<S> &Const) { return Const->i; }",
      isSmartPointerLikeOperatorArrow()));
  EXPECT_TRUE(matches(
      Decls, "int target(const HasGetAndValue<S> &Const) { return Const->i; }",
      isPointerLikeOperatorArrow()));

  EXPECT_TRUE(matches(
      Decls,
      "int target(HasGetAndValue<S> &NonConst) { return NonConst.value().i; }",
      isSmartPointerLikeValueMethodCall()));
  EXPECT_TRUE(matches(
      Decls,
      "int target(const HasGetAndValue<S> &Const) { return Const.value().i; }",
      isSmartPointerLikeValueMethodCall()));
  EXPECT_TRUE(matches(
      Decls,
      "int target(HasGetAndValue<S> &NonConst) { return NonConst.get()->i; }",
      isSmartPointerLikeGetMethodCall()));
  EXPECT_TRUE(matches(
      Decls,
      "int target(const HasGetAndValue<S> &Const) { return Const.get()->i; }",
      isSmartPointerLikeGetMethodCall()));
}

TEST(SmartPointerAccessorCachingTest, Renamed) {
  llvm::StringRef Decls(R"cc(
    namespace std {
    template <class T>
    struct unique_ptr {
      T* operator->() const;
      T& operator*() const;
      T* Get() const;
      T& Value() const;
    };
    }  // namespace std

    template <class T>
    using UniquePtrAlias = std::unique_ptr<T>;

    struct S { int i; };
  )cc");

  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S> P) { return (*P).i; }",
                      isPointerLikeOperatorStar()));

  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S> P) { return P->i; }",
                      isPointerLikeOperatorArrow()));

  EXPECT_TRUE(matches(Decls,
                      "int target(std::unique_ptr<S> P) { return P.Get()->i; }",
                      isSmartPointerLikeGetMethodCall("Get")));
  EXPECT_TRUE(matches(Decls,
                      "int target(UniquePtrAlias<S> P) { return P.Get()->i; }",
                      isSmartPointerLikeGetMethodCall("Get")));

  EXPECT_TRUE(
      matches(Decls, "int target(std::unique_ptr<S> P) { return P.Value().i; }",
              isSmartPointerLikeValueMethodCall("Value")));
  EXPECT_TRUE(matches(Decls,
                      "int target(UniquePtrAlias<S> P) { return P.Value().i; }",
                      isSmartPointerLikeValueMethodCall("Value")));

  EXPECT_TRUE(matches(Decls, "int target(UniquePtrAlias<S> P) { return P->i; }",
                      isPointerLikeOperatorArrow()));
}

} // namespace
} // namespace clang::dataflow
