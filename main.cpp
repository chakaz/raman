#include <array>
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

using std::array;
using std::map;
using std::string;
using std::unique_ptr;
using std::vector;

TEST_CASE("vector: ranged based for (copy)") {
  vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (auto i : lazy::From(in)) {
    out.push_back(i);
  }

  REQUIRE(in == out);
}

TEST_CASE("vector: ranged based for (const-ref)") {
  vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (const auto& i : lazy::From(in)) {
    out.push_back(i);
  }

  REQUIRE(in == out);
}

TEST_CASE("vector: old-style iteration") {
  vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  auto range = lazy::From(in);
  for (auto it = range.begin(), end = range.end(); it != end; ++it) {
    out.push_back(*it);
  }

  REQUIRE(in == out);
}

TEST_CASE("vector: ranged based for (mutate)") {
  vector<int> v = {1, 2, 3, 4, 5, 6, 7};

  for (auto& i : lazy::From(v)) {
    ++i;
  }

  REQUIRE(v == vector<int>{2, 3, 4, 5, 6, 7, 8});
}

TEST_CASE("vector: filter head") {
  vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (auto i : lazy::From(in).Where([](int i) { return i > 3; })) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{4, 5, 6, 7});
}

TEST_CASE("vector: filter tail") {
  vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (auto i : lazy::From(in).Where([](int i) { return i < 5; })) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{1, 2, 3, 4});
}

TEST_CASE("vector: filter head & tail") {
  vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (auto i : lazy::From(in).Where([](int i) { return i > 2 && i < 5; })) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{3, 4});
}

TEST_CASE("vector: filter head & tail (2 filters)") {
  vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (auto i :
      lazy::From(in)
        .Where([](int i) { return i > 2; })
        .Where([](int i) { return i < 5; })) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{3, 4});
}

TEST_CASE("vector: filter none") {
  vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (auto i : lazy::From(in).Where([](int i) { return true; })) {
    out.push_back(i);
  }

  REQUIRE(out == in);
}

TEST_CASE("vector: filter all") {
  vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (auto i : lazy::From(in).Where([](int i) { return false; })) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{});
}

TEST_CASE("vector: mutating with filter") {
  vector<int> v = {1, 2, 3, 4, 5, 6, 7};

  for (auto& i : lazy::From(v).Where([](int i) { return i > 4; })) {
    ++i;
  }

  REQUIRE(v == vector<int>{1, 2, 3, 4, 6, 7, 8});
}

TEST_CASE("vector: const container") {
  const vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (auto i : lazy::From(in)) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{1, 2, 3, 4, 5, 6, 7});
}

TEST_CASE("vector: transformation") {
  const vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (auto i : lazy::From(in).Transform([](int i) { return i+1; })) {
    out.push_back(i);
  }

  REQUIRE(in == vector<int>{1, 2, 3, 4, 5, 6, 7});
  REQUIRE(out == vector<int>{2, 3, 4, 5, 6, 7, 8});
}

TEST_CASE("vector: filter & transformation") {
  const vector<int> in = {1, 2, 3, 4, 5, 6, 7};
  vector<int> out;

  for (auto i :
      lazy::From(in)
        .Where([](int i) { return i > 3; })
        .Transform([](int i) { return i+1; })) {
    out.push_back(i);
  }

  REQUIRE(in == vector<int>{1, 2, 3, 4, 5, 6, 7});
  REQUIRE(out == vector<int>{5, 6, 7, 8});
}

TEST_CASE("map: keys") {
  const map<int, int> in = {{1, 2}, {3, 4}, {5, 6}};
  vector<int> out;

  for (auto i : lazy::From(in).Keys()) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{1, 3, 5});
}

TEST_CASE("map: values (const)") {
  const map<int, int> in = {{1, 2}, {3, 4}, {5, 6}};
  vector<int> out;

  for (auto i : lazy::From(in).Values()) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{2, 4, 6});
}

TEST_CASE("map: values (mutate)") {
  map<int, int> m = {{1, 2}, {3, 4}, {5, 6}};

  for (auto& i : lazy::From(m).Values()) {
    ++i;
  }

  REQUIRE(m == map<int, int>{{1, 3}, {3, 5}, {5, 7}});
}

TEST_CASE("vector: deref (const)") {
  array<int, 5> data = {0, 1, 2, 3, 4};
  vector<int*> in = {&data[0], &data[1], &data[2], &data[3], &data[4]};
  vector<int> out;

  for (auto i : lazy::From(in).Deref()) {
    out.push_back(i);
  }

  REQUIRE(out == std::vector<int>{0, 1, 2, 3, 4});
}

TEST_CASE("vector: deref (mutate)") {
  array<int, 5> data = {0, 1, 2, 3, 4};
  vector<int*> v = {&data[0], &data[1], &data[2], &data[3], &data[4]};

  for (auto& i : lazy::From(v).Deref()) {
    ++i;
  }

  REQUIRE(data == array<int, 5>{1, 2, 3, 4, 5});
}

TEST_CASE("vector: deref (unique_ptr)") {
  vector<unique_ptr<string>> v;
  v.push_back(std::make_unique<string>("0"));
  v.push_back(std::make_unique<string>("1"));
  v.push_back(std::make_unique<string>("2"));
  v.push_back(std::make_unique<string>("3"));
  v.push_back(std::make_unique<string>("4"));

  for (auto& s : lazy::From(v).Deref()) {
    s += "!";
  }

  REQUIRE(*v[0] == "0!");
  REQUIRE(*v[1] == "1!");
  REQUIRE(*v[2] == "2!");
  REQUIRE(*v[3] == "3!");
  REQUIRE(*v[4] == "4!");
}
