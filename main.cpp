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

  template <typename Container, typename Value = typename Container::value_type>
  vector<Value> ToVector(const Container& in) {
    vector<Value> out;
    std::copy(in.begin(), in.end(), std::back_inserter(out));
    return out;
  }

  template <typename Container, typename Value = typename Container::value_type>
  vector<Value> ToSortedVector(const Container& in) {
    vector<Value> out = ToVector(in);
    std::sort(out.begin(), out.end());
    return out;
  }
}

template <typename Container>
void RangedBasedForCopy(Container in) {
  Container out;

  for (typename Container::value_type i : lazy::From(in)) {
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

template <typename Container, typename Value = typename Container::value_type>
void TestSubRange(const Container& in) {
  auto begin = in.begin();
  ++begin;
  auto end = in.end();
  --end;
  vector<Value> out = lazy::From(begin, end);
  vector<Value> expected(begin, end);
  REQUIRE(out == expected);
}
TEST_CASE("sub-ranges") {
  TestSubRange(vector<int>{1, 2});
  TestSubRange(vector<int>{1, 2, 3});
  TestSubRange(vector<int>{1, 2, 3, 4});
  TestSubRange(list<int>{1, 2});
  TestSubRange(list<int>{1, 2, 3});
  TestSubRange(list<int>{1, 2, 3, 4});
  TestSubRange(deque<int>{1, 2});
  TestSubRange(deque<int>{1, 2, 3});
  TestSubRange(deque<int>{1, 2, 3, 4});
  TestSubRange(set<int>{1, 2});
  TestSubRange(set<int>{1, 2, 3});
  TestSubRange(set<int>{1, 2, 3, 4});
}

template <typename Container>
void RangedBasedForConstRef(Container in) {
  Container out;

  for (typename Container::const_reference i : lazy::From(in)) {
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
  for (typename Container::reference i : lazy::From(c)) {
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

  for (auto i : lazy::From(in).Dereference()) {
    out.push_back(i);
  }

  REQUIRE(out == std::vector<int>{0, 1, 2, 3, 4});
}

TEST_CASE("vector: deref (mutate)") {
  array<int, 5> data = {0, 1, 2, 3, 4};
  vector<int*> v = {&data[0], &data[1], &data[2], &data[3], &data[4]};

  for (auto& i : lazy::From(v).Dereference()) {
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

  for (auto& s : lazy::From(v).Dereference()) {
    s += "!";
  }

  REQUIRE(*v[0] == "0!");
  REQUIRE(*v[1] == "1!");
  REQUIRE(*v[2] == "2!");
  REQUIRE(*v[3] == "3!");
  REQUIRE(*v[4] == "4!");
}

TEST_CASE("vector: AddressOf (const)") {
  vector<int> in = {1, 2, 3, 4, 5};
  vector<int*> out;

  for (auto p : lazy::From(in).AddressOf()) {
    out.push_back(p);
  }

  REQUIRE(out[0] == &in[0]);
  REQUIRE(out[1] == &in[1]);
  REQUIRE(out[2] == &in[2]);
  REQUIRE(out[3] == &in[3]);
  REQUIRE(out[4] == &in[4]);
}

TEST_CASE("vector: AddressOf (mutate)") {
  vector<int> v = {1, 2, 3, 4, 5};

  for (auto p : lazy::From(v).AddressOf()) {
    *p = *p - 1;
  }

  REQUIRE(v == vector<int>{0, 1, 2, 3, 4});
}

TEST_CASE("vector: AddressOf, Dereference") {
  vector<int> v = {1, 2, 3, 4, 5};

  for (auto& p : lazy::From(v).AddressOf().Dereference()) {
    --p;
  }

  REQUIRE(v == vector<int>{0, 1, 2, 3, 4});
}

TEST_CASE("vector: rvalue") {
  auto factory = []() { return vector<int>{1, 2, 3, 4, 5}; };
  vector<int> out;

  for (auto i : lazy::From(factory())) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{1, 2, 3, 4, 5});
}

template <typename Container>
void SimpleReverse(Container in) {
  Container out;
  Container expected(in.rbegin(), in.rend());

  for (auto i : lazy::From(in).Reverse()) {
    AppendToContainer(out, i);
  }

  REQUIRE(out == expected);
}
TEST_CASE("simple reverse") {
  // int
  SimpleReverse(vector<int>{1, 2, 3, 4, 5, 6, 7});
  SimpleReverse(list<int>{1, 2, 3, 4, 5, 6, 7});
  SimpleReverse(deque<int>{1, 2, 3, 4, 5, 6, 7});

  // string
  SimpleReverse(vector<string>{"one", "two", "three"});
  SimpleReverse(list<string>{"one", "two", "three"});
  SimpleReverse(deque<string>{"one", "two", "three"});

  // empty
  SimpleReverse(vector<int>{});
  SimpleReverse(list<int>{});
  SimpleReverse(deque<int>{});
  SimpleReverse(vector<string>{});
  SimpleReverse(list<string>{});
  SimpleReverse(deque<string>{});

  // single element
  SimpleReverse(vector<int>{1});
  SimpleReverse(list<int>{1});
  SimpleReverse(deque<int>{1});
  SimpleReverse(vector<string>{"one"});
  SimpleReverse(list<string>{"one"});
  SimpleReverse(deque<string>{"one"});
}

TEST_CASE("vector: filter & reverse") {
  vector<int> in = {1, 2, 3, 4, 5, 6};
  vector<int> out;

  for (int i : lazy::From(in)
                     .Where([](int i) { return i > 2; })
                     .Reverse()) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{6, 5, 4, 3});
}

template <typename Container>
void EmptyRangeAllFeatures() {
  Container in, out;

  for (auto i : lazy::From(in)
                      .Reverse()
                      .Where([](int i) { return i > 2; })
                      .Reverse()
                      .Where([](int i) { return i < 20; })
                      .AddressOf()
                      .Dereference()
                      .Sort()
                      .Reverse()) {
    AppendToContainer(out, i);
  }

  REQUIRE(out == Container());
}
TEST_CASE("empty range") {
  EmptyRangeAllFeatures<vector<int>>();
  EmptyRangeAllFeatures<list<int>>();
  EmptyRangeAllFeatures<deque<int>>();
  EmptyRangeAllFeatures<set<int>>();
}

TEST_CASE("vector: all features") {
  vector<int> in = {1, 3, 2, 5, 6, 4, 7, 8, 9, 10};
  vector<int> out;

  for (int i : lazy::From(in)
                     .Where([](int i) { return i > 2; })
                     .Sort()
                     .Reverse()
                     .AddressOf()
                     .Dereference()
                     .Transform([](int i) { return i * 2; })
                     .Where([](int i) { return i < 18; })
                     .Reverse()) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{6, 8, 10, 12, 14, 16});
}

struct FilterSmallerThan2 {
  bool operator()(int i) const { return i > 2; }
  bool operator==(const FilterSmallerThan2& o) const { return true; }
};
TEST_CASE("vector: non-lambda filter") {
  vector<int> in = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  vector<int> out;

  for (int i : lazy::From(in)
                     .Where(FilterSmallerThan2())) {
    out.push_back(i);
  }

  REQUIRE(out == vector<int>{3, 4, 5, 6, 7, 8, 9, 10});
}

template <typename Container>
void TestSort(const Container& container) {
  Container in = container;
  Container out;

  Container expectedOut = container;
  std::sort(expectedOut.begin(), expectedOut.end());
  Container expectedModified = container;
  for (auto& value : expectedModified) {
    value += value;
  }

  for (auto& value : lazy::From(in)
                           .Sort()) {
    AppendToContainer(out, value);
    value += value;
  }

  REQUIRE(in == expectedModified);
  REQUIRE(out == expectedOut);
}
TEST_CASE("Sort") {
  TestSort<vector<int>>({1, 3, 2, 5, 4});
  TestSort<deque<int>>({1, 3, 2, 5, 4});
  TestSort<vector<string>>({"1=one", "3=three", "2=two"});
  TestSort<deque<string>>({"1=one", "3=three", "2=two"});
}

template <typename ContainerTo, typename ContainerFrom>
void TestCastToContainer(const ContainerFrom& in) {
  ContainerTo out = lazy::From(in);
  REQUIRE(ToVector(out) == ToVector(in));
}
template <typename ContainerTo, typename ContainerFrom>
void TestCastToContainerUnordered(const ContainerFrom& in) {
  ContainerTo out = lazy::From(in);
  REQUIRE(ToSortedVector(out) == ToSortedVector(in));
}
TEST_CASE("Cast to container") {
  vector<int> v = lazy::From(vector<int>{1, 3, 2, 4, 5}).Sort().Reverse();
  REQUIRE(v == vector<int>{5, 4, 3, 2, 1});

  // To vector
  TestCastToContainer<vector<int>>(vector<int>{1, 2, 3, 4});
  TestCastToContainer<vector<int>>(list<int>{1, 2, 3, 4});
  TestCastToContainer<vector<int>>(deque<int>{1, 2, 3, 4});
  TestCastToContainer<vector<int>>(set<int>{1, 2, 3, 4});
  TestCastToContainerUnordered<vector<int>>(unordered_set<int>{1, 2, 3, 4});
  TestCastToContainer<vector<string>>(vector<string>{"one", "two"});
  TestCastToContainer<vector<string>>(list<string>{"one", "two"});
  TestCastToContainer<vector<string>>(deque<string>{"one", "two"});
  TestCastToContainer<vector<string>>(set<string>{"one", "two"});
  TestCastToContainerUnordered<vector<string>>(unordered_set<string>{"one", "two"});

  // To list
  TestCastToContainer<list<int>>(vector<int>{1, 2, 3, 4});
  TestCastToContainer<list<int>>(list<int>{1, 2, 3, 4});
  TestCastToContainer<list<int>>(deque<int>{1, 2, 3, 4});
  TestCastToContainer<list<int>>(set<int>{1, 2, 3, 4});
  TestCastToContainerUnordered<list<int>>(unordered_set<int>{1, 2, 3, 4});
  TestCastToContainer<list<string>>(vector<string>{"one", "two"});
  TestCastToContainer<list<string>>(list<string>{"one", "two"});
  TestCastToContainer<list<string>>(deque<string>{"one", "two"});
  TestCastToContainer<list<string>>(set<string>{"one", "two"});
  TestCastToContainerUnordered<list<string>>(unordered_set<string>{"one", "two"});

  // To deque
  TestCastToContainer<deque<int>>(vector<int>{1, 2, 3, 4});
  TestCastToContainer<deque<int>>(list<int>{1, 2, 3, 4});
  TestCastToContainer<deque<int>>(deque<int>{1, 2, 3, 4});
  TestCastToContainer<deque<int>>(set<int>{1, 2, 3, 4});
  TestCastToContainerUnordered<deque<int>>(unordered_set<int>{1, 2, 3, 4});
  TestCastToContainer<deque<string>>(vector<string>{"one", "two"});
  TestCastToContainer<deque<string>>(list<string>{"one", "two"});
  TestCastToContainer<deque<string>>(deque<string>{"one", "two"});
  TestCastToContainer<deque<string>>(set<string>{"one", "two"});
  TestCastToContainerUnordered<deque<string>>(unordered_set<string>{"one", "two"});

  // To set
  TestCastToContainer<set<int>>(vector<int>{1, 2, 3, 4});
  TestCastToContainer<set<int>>(list<int>{1, 2, 3, 4});
  TestCastToContainer<set<int>>(deque<int>{1, 2, 3, 4});
  TestCastToContainer<set<int>>(set<int>{1, 2, 3, 4});
  TestCastToContainerUnordered<set<int>>(unordered_set<int>{1, 2, 3, 4});
  TestCastToContainer<set<string>>(vector<string>{"one", "two"});
  TestCastToContainer<set<string>>(list<string>{"one", "two"});
  TestCastToContainer<set<string>>(deque<string>{"one", "two"});
  TestCastToContainer<set<string>>(set<string>{"one", "two"});
  TestCastToContainerUnordered<set<string>>(unordered_set<string>{"one", "two"});

  // To unordered_set
  TestCastToContainerUnordered<unordered_set<int>>(vector<int>{1, 2, 3, 4});
  TestCastToContainerUnordered<unordered_set<int>>(list<int>{1, 2, 3, 4});
  TestCastToContainerUnordered<unordered_set<int>>(deque<int>{1, 2, 3, 4});
  TestCastToContainerUnordered<unordered_set<int>>(set<int>{1, 2, 3, 4});
  TestCastToContainerUnordered<unordered_set<int>>(unordered_set<int>{1, 2, 3, 4});
  TestCastToContainerUnordered<unordered_set<string>>(vector<string>{"1", "2"});
  TestCastToContainerUnordered<unordered_set<string>>(list<string>{"1", "2"});
  TestCastToContainerUnordered<unordered_set<string>>(deque<string>{"1", "2"});
  TestCastToContainerUnordered<unordered_set<string>>(set<string>{"1", "2"});
  TestCastToContainerUnordered<unordered_set<string>>(unordered_set<string>{"1", "2"});
}

// TODO: test:
// - const ranges
// - ranges with const elements
// - const ranges with const elements
// - From(initializer-list)
