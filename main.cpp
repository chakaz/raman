#include <array>
#include <functional>
#include <iostream>
#include <list>
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
using std::deque;
using std::list;
using std::map;
using std::set;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

namespace {
  template <typename Container, typename Value>
  void AppendToContainer(Container& container, Value&& value) {
    container.insert(container.end(), std::forward<Value>(value));
  }
}

template <typename Container>
void RangedBasedForCopy(Container in) {
  Container out;

  for (auto i : lazy::From(in)) {
    AppendToContainer(out, i);
  }

  REQUIRE(in == out);
}

TEST_CASE("ranged based for (copy)") {
  // int
  RangedBasedForCopy(vector<int>{1, 2, 3, 4, 5, 6, 7});
  RangedBasedForCopy(list<int>{1, 2, 3, 4, 5, 6, 7});
  RangedBasedForCopy(deque<int>{1, 2, 3, 4, 5, 6, 7});
  RangedBasedForCopy(set<int>{1, 2, 3, 4, 5, 6, 7});
  RangedBasedForCopy(unordered_set<int>{1, 2, 3, 4, 5, 6, 7});
  RangedBasedForCopy(map<int, int>{{1, 1}, {2, 2}, {3, 3}, {4, 4}});
  RangedBasedForCopy(unordered_map<int, int>{{1, 1}, {2, 2}, {3, 3}, {4, 4}});

  // string
  RangedBasedForCopy(vector<string>{"one", "two", "three"});
  RangedBasedForCopy(list<string>{"one", "two", "three"});
  RangedBasedForCopy(deque<string>{"one", "two", "three"});
  RangedBasedForCopy(set<string>{"one", "two", "three"});
  RangedBasedForCopy(unordered_set<string>{"one", "two", "three"});
  RangedBasedForCopy(map<string, string>{{"one", "1"}, {"two", "2"}});
  RangedBasedForCopy(unordered_map<string, string>{{"one", "1"}, {"two", "2"}});
}

TEST_CASE("empty ranges") {
  // int
  RangedBasedForCopy(vector<int>());
  RangedBasedForCopy(list<int>());
  RangedBasedForCopy(deque<int>());
  RangedBasedForCopy(set<int>());
  RangedBasedForCopy(unordered_set<int>());
  RangedBasedForCopy(map<int, int>());
  RangedBasedForCopy(unordered_map<int, int>());

  // string
  RangedBasedForCopy(vector<string>());
  RangedBasedForCopy(list<string>());
  RangedBasedForCopy(deque<string>());
  RangedBasedForCopy(set<string>());
  RangedBasedForCopy(unordered_set<string>());
  RangedBasedForCopy(map<string, string>());
  RangedBasedForCopy(unordered_map<string, string>());
}

template <typename Container>
void RangedBasedForConstRef(Container in) {
  Container out;

  for (const auto& i : lazy::From(in)) {
    AppendToContainer(out, i);
  }

  REQUIRE(in == out);
}

TEST_CASE("ranged based for (const-ref)") {
  // int
  RangedBasedForConstRef(vector<int>{1, 2, 3, 4, 5, 6, 7});
  RangedBasedForConstRef(list<int>{1, 2, 3, 4, 5, 6, 7});
  RangedBasedForConstRef(deque<int>{1, 2, 3, 4, 5, 6, 7});
  RangedBasedForConstRef(set<int>{1, 2, 3, 4, 5, 6, 7});
  RangedBasedForConstRef(unordered_set<int>{1, 2, 3, 4, 5, 6, 7});
  RangedBasedForConstRef(map<int, int>{{1, 1}, {2, 2}, {3, 3}, {4, 4}});
  RangedBasedForConstRef(unordered_map<int, int>{{1, 1}, {2, 2}, {3, 3}, {4, 4}});

  // string
  RangedBasedForConstRef(vector<string>{"one", "two", "three"});
  RangedBasedForConstRef(list<string>{"one", "two", "three"});
  RangedBasedForConstRef(deque<string>{"one", "two", "three"});
  RangedBasedForConstRef(set<string>{"one", "two", "three"});
  RangedBasedForConstRef(unordered_set<string>{"one", "two", "three"});
  RangedBasedForConstRef(map<string, string>{{"one", "1"}, {"two", "2"}});
  RangedBasedForConstRef(unordered_map<string, string>{{"one", "1"}, {"two", "2"}});
}

template <typename Container>
void OldStyleFor(Container in) {
  Container out;

  auto range = lazy::From(in);
  for (auto it = range.begin(), end = range.end(); it != end; ++it) {
    AppendToContainer(out, *it);
  }

  REQUIRE(in == out);
}

TEST_CASE("old-style for") {
  // int
  OldStyleFor(vector<int>{1, 2, 3, 4, 5, 6, 7});
  OldStyleFor(list<int>{1, 2, 3, 4, 5, 6, 7});
  OldStyleFor(deque<int>{1, 2, 3, 4, 5, 6, 7});
  OldStyleFor(set<int>{1, 2, 3, 4, 5, 6, 7});
  OldStyleFor(unordered_set<int>{1, 2, 3, 4, 5, 6, 7});
  OldStyleFor(map<int, int>{{1, 1}, {2, 2}, {3, 3}, {4, 4}});
  OldStyleFor(unordered_map<int, int>{{1, 1}, {2, 2}, {3, 3}, {4, 4}});

  // string
  OldStyleFor(vector<string>{"one", "two", "three"});
  OldStyleFor(list<string>{"one", "two", "three"});
  OldStyleFor(deque<string>{"one", "two", "three"});
  OldStyleFor(set<string>{"one", "two", "three"});
  OldStyleFor(unordered_set<string>{"one", "two", "three"});
  OldStyleFor(map<string, string>{{"one", "1"}, {"two", "2"}});
  OldStyleFor(unordered_map<string, string>{{"one", "1"}, {"two", "2"}});
}

template <typename Container>
void RangedBasedForMutating(Container in, Container expected) {
  Container c = in;
  for (auto& i : lazy::From(c)) {
    i = i + i;
  }

  REQUIRE(c == expected);
}

TEST_CASE("ranged based for (mutate)") {
  // int
  RangedBasedForMutating(vector<int>{1, 2, 3, 4, 5},
                         vector<int>{2, 4, 6, 8, 10});
  RangedBasedForMutating(list<int>{1, 2, 3, 4, 5},
                         list<int>{2, 4, 6, 8, 10});
  RangedBasedForMutating(deque<int>{1, 2, 3, 4, 5},
                         deque<int>{2, 4, 6, 8, 10});

  // string
  RangedBasedForMutating(vector<string>{"one", "two", "three"},
                         vector<string>{"oneone", "twotwo", "threethree"});
  RangedBasedForMutating(list<string>{"one", "two", "three"},
                         list<string>{"oneone", "twotwo", "threethree"});
  RangedBasedForMutating(deque<string>{"one", "two", "three"},
                         deque<string>{"oneone", "twotwo", "threethree"});
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
