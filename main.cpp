#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "lazy.hpp"

TEST_CASE("vector: ranged based for (copy)") {
  std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (auto i : lazy::From(in)) {
    out.push_back(i);
  }

  REQUIRE(in == out);
}

TEST_CASE("vector: ranged based for (const-ref)") {
  std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (const auto& i : lazy::From(in)) {
    out.push_back(i);
  }

  REQUIRE(in == out);
}

TEST_CASE("vector: old-style iteration") {
  std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  auto range = lazy::From(in);
  for (auto it = range.begin(), end = range.end(); it != end; ++it) {
    out.push_back(*it);
  }

  REQUIRE(in == out);
}

TEST_CASE("vector: ranged based for (mutate)") {
  std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

  for (auto& i : lazy::From(v)) {
    ++i;
  }

  REQUIRE(v == std::vector<int>{2, 3, 4, 5, 6, 7, 8});
}

TEST_CASE("vector: filter head") {
  std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (auto i : lazy::From(in).Where([](int i) { return i > 3; })) {
    out.push_back(i);
  }

  REQUIRE(out == std::vector<int>{4, 5, 6, 7});
}

TEST_CASE("vector: filter tail") {
  std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (auto i : lazy::From(in).Where([](int i) { return i < 5; })) {
    out.push_back(i);
  }

  REQUIRE(out == std::vector<int>{1, 2, 3, 4});
}

TEST_CASE("vector: filter head & tail") {
  std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (auto i : lazy::From(in).Where([](int i) { return i > 2 && i < 5; })) {
    out.push_back(i);
  }

  REQUIRE(out == std::vector<int>{3, 4});
}

TEST_CASE("vector: filter head & tail (2 filters)") {
  std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (auto i :
      lazy::From(in)
        .Where([](int i) { return i > 2; })
        .Where([](int i) { return i < 5; })) {
    out.push_back(i);
  }

  REQUIRE(out == std::vector<int>{3, 4});
}

TEST_CASE("vector: filter none") {
  std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (auto i : lazy::From(in).Where([](int i) { return true; })) {
    out.push_back(i);
  }

  REQUIRE(out == in);
}

TEST_CASE("vector: filter all") {
  std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (auto i : lazy::From(in).Where([](int i) { return false; })) {
    out.push_back(i);
  }

  REQUIRE(out == std::vector<int>{});
}

TEST_CASE("vector: mutating with filter") {
  std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

  for (auto& i : lazy::From(v).Where([](int i) { return i > 4; })) {
    ++i;
  }

  REQUIRE(v == std::vector<int>{1, 2, 3, 4, 6, 7, 8});
}

TEST_CASE("vector: const container") {
  const std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (auto i : lazy::From(in)) {
    out.push_back(i);
  }

  REQUIRE(out == std::vector<int>{1, 2, 3, 4, 5, 6, 7});
}

TEST_CASE("vector: transformation") {
  const std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (auto i : lazy::From(in).Transform([](int i) { return i+1; })) {
    out.push_back(i);
  }

  REQUIRE(in == std::vector<int>{1, 2, 3, 4, 5, 6, 7});
  REQUIRE(out == std::vector<int>{2, 3, 4, 5, 6, 7, 8});
}

TEST_CASE("vector: filter & transformation") {
  const std::vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  std::vector<int> out;

  for (auto i :
      lazy::From(in)
        .Where([](int i) { return i > 3; })
        .Transform([](int i) { return i+1; })) {
    out.push_back(i);
  }

  REQUIRE(in == std::vector<int>{1, 2, 3, 4, 5, 6, 7});
  REQUIRE(out == std::vector<int>{5, 6, 7, 8});
}

TEST_CASE("map: keys") {
  const std::map<int, int> in = {{1, 2}, {3, 4}, {5, 6}};
  std::vector<int> out;

  for (auto i : lazy::From(in).Keys()) {
    out.push_back(i);
  }

  REQUIRE(out == std::vector<int>{1, 3, 5});
}

TEST_CASE("map: values (const)") {
  const std::map<int, int> in = {{1, 2}, {3, 4}, {5, 6}};
  std::vector<int> out;

  for (auto i : lazy::From(in).Values()) {
    out.push_back(i);
  }

  REQUIRE(out == std::vector<int>{2, 4, 6});
}

TEST_CASE("map: values (mutate)") {
  std::map<int, int> m = {{1, 2}, {3, 4}, {5, 6}};

  for (auto& i : lazy::From(m).Values()) {
    ++i;
  }

  REQUIRE(m == std::map<int, int>{{1, 3}, {3, 5}, {5, 7}});
}
