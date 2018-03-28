#ifndef LAZY_CONTAINERS_LIBRARY
#define LAZY_CONTAINERS_LIBRARY

#include <iterator>
#include <memory>
#include <type_traits>

// TODO: this really needs to be better.
#define ASSERT(x)                     \
  if (!(x)) {                         \
    ::lazy::internal::DieDebugHook(); \
    throw 123;                        \
  }

// Solve:
// - Force telescoping
// - Allow forward (i.e: non-backward) iterators
namespace lazy {
  namespace internal {
    void DieDebugHook() {}

    template <typename Container>
    using IteratorOf =
        typename std::decay<decltype(std::declval<Container>().begin())>::type;

    template <typename T>
    constexpr bool IsAssignable() {
      return std::is_copy_assignable<T>::value;
    }

    // Horrible, horrible hack to allow copy/move assignment of functors, which
    // prior to C++20 can't be assigned.
    template <typename Functor, typename = void>
    struct AssignableFunctor;

    template <typename Functor>
    struct AssignableFunctor<
        Functor,
        typename std::enable_if<IsAssignable<Functor>()>::type> {
      AssignableFunctor(Functor functor_arg)
          : functor(std::move(functor_arg)) {}

      AssignableFunctor(const AssignableFunctor&) = default;
      AssignableFunctor(AssignableFunctor&&) = default;
      AssignableFunctor& operator=(const AssignableFunctor& o) = default;
      AssignableFunctor& operator=(AssignableFunctor&& o) = default;

      Functor functor;
    };

    template <typename Functor>
    struct AssignableFunctor<
        Functor,
        typename std::enable_if<!IsAssignable<Functor>()>::type> {
      AssignableFunctor(Functor functor_arg)
          : functor(std::move(functor_arg)) {}

      AssignableFunctor(const AssignableFunctor&) = default;
      AssignableFunctor(AssignableFunctor&&) = default;
      AssignableFunctor& operator=(const AssignableFunctor& o) {
        functor.~Functor();
        new (&functor) Functor(o.functor);
        return *this;
      }
      AssignableFunctor& operator=(AssignableFunctor&& o) {
        functor.~Functor();
        new (&functor) Functor(std::move(o.functor));
        return *this;
      }

      Functor functor;
    };

    template <typename Iterator>
    struct SimpleRange {
      explicit SimpleRange(Iterator begin, Iterator end)
          : begin_(std::move(begin)),
            current_(begin_),
            end_(std::move(end)) {}

      SimpleRange(const SimpleRange&) = default;
      SimpleRange& operator=(const SimpleRange&) = default;
      SimpleRange(SimpleRange&&) = default;
      SimpleRange& operator=(SimpleRange&&) = default;

      // Iterator typedefs.
      using iterator_category = typename Iterator::iterator_category;
      using value_type = typename Iterator::value_type;
      using difference_type = typename Iterator::difference_type;
      using pointer = typename Iterator::pointer;
      using reference = typename Iterator::reference;

      decltype(auto) CurrentValue() const {
        ASSERT(!IsAtEnd());
        return *current_;
      }

      void GoToBegin() {
        current_ = begin_;
      }

      void GoToEnd() {
        current_ = end_;
      }

      void Advance() {
        ASSERT(current_ != end_);
        ++current_;
      }

      void Retreat() {
        ASSERT(current_ != begin_);
        --current_;
      }

      bool operator==(const SimpleRange& o) const {
        return (begin_ == o.begin_ &&
                end_ == o.end_ &&
                current_ == o.current_);
      }

      bool IsAtBegin() const {
        return current_ == begin_;
      }

      bool IsAtEnd() const {
        return current_ == end_;
      }

      Iterator begin_;  // Non-const to support copy-assignment.
      Iterator current_;
      Iterator end_;  // Non-const to support copy-assignment.
    };

    template <typename Container>
    struct ContainerOwner {
      explicit ContainerOwner(Container&& container)
          : container_(std::move(container)) {}

     protected:
      Container container_;
    };

    // Like SimpleRange, but also owns the container. Built for rvalues.
    template <typename Container>
    struct SimpleRangeOwner : ContainerOwner<Container>,
                              SimpleRange<IteratorOf<Container>> {
      explicit SimpleRangeOwner(Container&& container)
          : ContainerOwner<Container>(std::move(container)),
            SimpleRange<IteratorOf<Container>>(
                ContainerOwner<Container>::container_.begin(),
                ContainerOwner<Container>::container_.end()) {}

      SimpleRangeOwner(const SimpleRangeOwner&) = default;
      SimpleRangeOwner& operator=(const SimpleRangeOwner&) = default;
      SimpleRangeOwner(SimpleRangeOwner&&) = default;
      SimpleRangeOwner& operator=(SimpleRangeOwner&&) = default;
    };

    // Filtered range.
    template <typename Range, typename Filter>
    struct FilteredRange {
      explicit FilteredRange(Range range, Filter filter)
          : range_(std::move(range)),
            unfiltered_begin_(range_),
            filter_(std::move(filter)) {
        range_.GoToBegin();
        AdvanceToNextNonFilteredIfNeeded();
        unfiltered_begin_ = range_;
      }

      FilteredRange(const FilteredRange&) = default;
      FilteredRange& operator=(const FilteredRange&) = default;
      FilteredRange(FilteredRange&&) = default;
      FilteredRange& operator=(FilteredRange&&) = default;

      // Iterator typedefs.
      using iterator_category = typename Range::iterator_category;
      using value_type = typename Range::value_type;
      using difference_type = typename Range::difference_type;
      using pointer = typename Range::pointer;
      using reference = typename Range::reference;

      decltype(auto) CurrentValue() const {
        return range_.CurrentValue();
      }

      void GoToBegin() {
        range_ = unfiltered_begin_;
      }

      void GoToEnd() {
        range_.GoToEnd();
      }

      void Advance() {
        range_.Advance();
        AdvanceToNextNonFilteredIfNeeded();
      }

      void Retreat() {
        range_.Retreat();
        RetreatToPreviousNonFilteredIfNeeded();
      }

      bool operator==(const FilteredRange& o) const {
        return (range_ == o.range_ && filter_.functor == o.filter_.functor);
      }

      bool IsAtBegin() const {
        return range_ == unfiltered_begin_;
      }

      bool IsAtEnd() const {
        return range_.IsAtEnd();
      }

     private:
      // Will not advance if current element is not filtered.
      void AdvanceToNextNonFilteredIfNeeded() {
        while (!IsAtEnd() && !filter_.functor(range_.CurrentValue())) {
          range_.Advance();
        }
      }

      void RetreatToPreviousNonFilteredIfNeeded() {
        while (!IsAtBegin() && !filter_.functor(range_.CurrentValue())) {
          range_.Retreat();
        }
      }

      Range range_;
      Range unfiltered_begin_;
      AssignableFunctor<Filter> filter_;
    };

    template <typename Range>
    struct SimpleRangeWrapper {
      explicit SimpleRangeWrapper(Range range)
          : range_(std::move(range)) {}

      SimpleRangeWrapper(const SimpleRangeWrapper&) = default;
      SimpleRangeWrapper& operator=(const SimpleRangeWrapper&) = default;
      SimpleRangeWrapper(SimpleRangeWrapper&&) = default;
      SimpleRangeWrapper& operator=(SimpleRangeWrapper&&) = default;

      // Iterator typedefs.
      using iterator_category = typename Range::iterator_category;
      using value_type = typename Range::value_type;
      using difference_type = typename Range::difference_type;
      using pointer = typename Range::pointer;
      using reference = typename Range::reference;

      void GoToBegin() {
        range_.GoToBegin();
      }

      void GoToEnd() {
        range_.GoToEnd();
      }

      void Advance() {
        range_.Advance();
      }

      void Retreat() {
        range_.Retreat();
      }

      bool IsAtBegin() const {
        return range_.IsAtBegin();
      }

      bool IsAtEnd() const {
        return range_.IsAtEnd();
      }

     protected:
      Range range_;
    };

    template <typename Range, typename Transformer>
    struct ByValueTransformerRange : SimpleRangeWrapper<Range> {
      explicit ByValueTransformerRange(Range range, Transformer transformer)
          : SimpleRangeWrapper<Range>(std::move(range)),
            transformer_(std::move(transformer)) {}

      ByValueTransformerRange(const ByValueTransformerRange&) = default;
      ByValueTransformerRange& operator=(const ByValueTransformerRange&) = default;
      ByValueTransformerRange(ByValueTransformerRange&&) = default;
      ByValueTransformerRange& operator=(ByValueTransformerRange&&) = default;

      auto CurrentValue() const {
        return transformer_.functor(this->range_.CurrentValue());
      }

      bool operator==(const ByValueTransformerRange& o) const {
        return (this->range_ == o.range_ &&
                transformer_.functor == o.transformer_.functor);
      }

     private:
      AssignableFunctor<Transformer> transformer_;
    };

    // Transformer range.
    template <typename Range, typename Transformer>
    struct ByRefTransformerRange : SimpleRangeWrapper<Range> {
      explicit ByRefTransformerRange(Range range, Transformer transformer)
          : SimpleRangeWrapper<Range>(std::move(range)),
            transformer_(std::move(transformer)) {}

      ByRefTransformerRange(const ByRefTransformerRange&) = default;
      ByRefTransformerRange& operator=(const ByRefTransformerRange&) = default;
      ByRefTransformerRange(ByRefTransformerRange&&) = default;
      ByRefTransformerRange& operator=(ByRefTransformerRange&&) = default;

      decltype(auto) CurrentValue() const {
        return transformer_.functor(this->range_.CurrentValue());
      }

      bool operator==(const ByRefTransformerRange& o) const {
        return (this->range_ == o.range_ &&
                transformer_.functor == o.transformer_.functor);
      }

     private:
      AssignableFunctor<Transformer> transformer_;
    };

    template <typename Range>
    struct ReverseRange {
      explicit ReverseRange(Range range)
          : range_(std::move(range)),
            last_element_(range_) {
        last_element_.GoToEnd();
        if (!last_element_.IsAtBegin()) {
          last_element_.Retreat();
        }
        this->GoToBegin();
      }

      ReverseRange(const ReverseRange&) = default;
      ReverseRange& operator=(const ReverseRange&) = default;
      ReverseRange(ReverseRange&&) = default;
      ReverseRange& operator=(ReverseRange&&) = default;

      // Iterator typedefs.
      using iterator_category = typename Range::iterator_category;
      using value_type = typename Range::value_type;
      using difference_type = typename Range::difference_type;
      using pointer = typename Range::pointer;
      using reference = typename Range::reference;

      decltype(auto) CurrentValue() const {
        ASSERT(!IsAtEnd());
        return range_.CurrentValue();
      }

      void GoToBegin() {
        range_ = last_element_;
        is_at_rend_ = range_.IsAtBegin();
      }

      void GoToEnd() {
        range_.GoToBegin();
        is_at_rend_ = true;
      }

      void Advance() {
        ASSERT(!IsAtEnd());
        if (range_.IsAtBegin()) {
          is_at_rend_ = true;
        } else {
          range_.Retreat();
        }
      }

      void Retreat() {
        ASSERT(!IsAtBegin());
        if (is_at_rend_) {
          is_at_rend_ = false;
        } else {
          range_.Advance();
        }
      }

      bool operator==(const ReverseRange& o) const {
        return (range_ == o.range_ &&
                is_at_rend_ == o.is_at_rend_ &&
                last_element_ == o.last_element_);
      }

      bool IsAtBegin() const {
        return range_ == last_element_;
      }

      bool IsAtEnd() const {
        return is_at_rend_;
      }

     private:
      Range range_;

      // Range in a state pointing to one before end if exists.
      Range last_element_;

      // Are we at one-before-begin bit.
      bool is_at_rend_ = false;
    };

    template <typename Range>
    struct RangeIterator {
      using iterator_category = typename Range::iterator_category;
      using value_type = typename Range::value_type;
      using difference_type = typename Range::difference_type;
      using pointer = typename Range::pointer;
      using reference = typename Range::reference;

      explicit RangeIterator(Range range, bool is_end)
          : range_(std::move(range)) {
        if (is_end) {
          range_.GoToEnd();
        }
      }

      RangeIterator(const RangeIterator&) = default;
      RangeIterator& operator=(const RangeIterator&) = default;
      RangeIterator(RangeIterator&&) = default;
      RangeIterator& operator=(RangeIterator&&) = default;

      decltype(auto) operator*() const {
        return range_.CurrentValue();
      }

      auto operator->() const {
        return &range_.CurrentValue();
      }

      RangeIterator& operator++() {
        range_.Advance();
        return *this;
      }

      RangeIterator operator++(int) {
        RangeIterator it = *this;
        range_.Advance();
        return it;
      }

      RangeIterator& operator--() {
        range_.Retreat();
        return *this;
      }

      RangeIterator operator--(int) {
        RangeIterator it = *this;
        range_.Retreat();
        return it;
      }

      bool operator==(const RangeIterator& o) const {
        return range_ == o.range_;
      }

      bool operator!=(const RangeIterator& o) const {
        return !(*this == o);
      }

     private:
      Range range_;
    };

    template <typename Range>
    struct LazyWrapper {
      explicit LazyWrapper(Range range)
          : range_(std::move(range)) {}

      LazyWrapper(const LazyWrapper&) = default;
      LazyWrapper& operator=(const LazyWrapper&) = default;
      LazyWrapper(LazyWrapper&&) = default;
      LazyWrapper& operator=(LazyWrapper&&) = default;

      template <typename Filter>
      auto Where(Filter filter) {
        using InnerRange = FilteredRange<Range, Filter>;
        return LazyWrapper<InnerRange>(InnerRange(range_, std::move(filter)));
      }

      template <typename Transformer>
      auto Transform(Transformer transformer) {
        using InnerRange = ByValueTransformerRange<Range, Transformer>;
        return LazyWrapper<InnerRange>(
            InnerRange(range_, std::move(transformer)));
      }

      auto Keys() {
        using Entry = decltype(std::declval<Range>().CurrentValue());
        auto transformer = [](const Entry& entry) { return entry.first; };
        using InnerRange = ByValueTransformerRange<Range, decltype(transformer)>;
        return LazyWrapper<InnerRange>(
            InnerRange(range_, std::move(transformer)));
      }

      auto Values() {
        using Entry = decltype(std::declval<Range>().CurrentValue());
        auto transformer = [](const Entry& entry) -> auto& {
          return entry.second;
        };
        using InnerRange = ByRefTransformerRange<Range, decltype(transformer)>;
        return LazyWrapper<InnerRange>(
            InnerRange(range_, std::move(transformer)));
      }

      auto Dereference() {
        using Entry = decltype(std::declval<Range>().CurrentValue());
        auto transformer = [](Entry entry) -> auto& { return *entry; };
        using InnerRange = ByRefTransformerRange<Range, decltype(transformer)>;
        return LazyWrapper<InnerRange>(
            InnerRange(range_, std::move(transformer)));
      }

      auto AddressOf() {
        using Entry = decltype(std::declval<Range>().CurrentValue());
        auto transformer = [](Entry& entry) { return &entry; };
        using InnerRange = ByValueTransformerRange<Range, decltype(transformer)>;
        return LazyWrapper<InnerRange>(
            InnerRange(range_, std::move(transformer)));
      }

      auto Reverse() {
        using InnerRange = ReverseRange<Range>;
        return LazyWrapper<InnerRange>(InnerRange(range_));
      }

      // TODO: print what's wrong in ASSERT()
      // TODO: OrderBy()
      // TODO: SkipRepeating()
      // TODO: OrderByUnique()

      auto begin() {
        return RangeIterator<Range>(range_, /*is_end=*/false);
      }

      auto begin() const {
        return RangeIterator<const Range>(range_, /*is_end=*/false);
      }

      auto end() {
        return RangeIterator<Range>(range_, /*is_end=*/true);
      }

      auto end() const {
        return RangeIterator<const Range>(range_, /*is_end=*/true);
      }

     private:
      Range range_;
    };
  }

  template <typename Iterator>
  auto From(Iterator begin, Iterator end) {
    using Range = internal::SimpleRange<Iterator>;
    return internal::LazyWrapper<Range>(
        Range(std::move(begin), std::move(end)));
  }

  // This version takes objects with lifetimes longer than the wrapper.
  template <typename Container>
  auto From(Container& container) {
    return From(container.begin(), container.end());
  }

  // This version keeps `container` alive while the wrapper is alive.
  template <typename Container>
  auto From(Container&& container) {
    using Range = internal::SimpleRangeOwner<Container>;
    return internal::LazyWrapper<Range>(Range(std::move(container)));
  }
}

#endif  //LAZY_CONTAINERS_LIBRARY
