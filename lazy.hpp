#ifndef LAZY_CONTAINERS_LIBRARY
#define LAZY_CONTAINERS_LIBRARY

// TODO: this really needs to be better.
#define ASSERT(x) if (!(x)) { throw 123; }

// Solve:
// - Force telescoping
// - Store container?
// - const/mutable values
// - std::move everything?
namespace lazy {
  namespace internal {
    template <typename Iterator>
    struct SimpleRange {
      SimpleRange(Iterator begin, Iterator end)
          : begin_(std::move(begin)),
            current_(std::move(begin_)),
            end_(std::move(end)) {}

      SimpleRange(const SimpleRange&) = default;
      SimpleRange& operator=(const SimpleRange&) = default;
      SimpleRange(SimpleRange&&) = default;
      SimpleRange& operator=(SimpleRange&&) = default;

      auto& CurrentValue() const {
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

      bool Equals(const SimpleRange& o) const {
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

      const Iterator begin_;
      Iterator current_;
      const Iterator end_;
    };

    // Filtered range.
    template <typename Range, typename Filter>
    struct FilteredRange {
      FilteredRange(Range range, Filter filter)
          : range_(std::move(range)),
            filter_(std::move(filter)) {
        AdvanceToNextNonFilteredIfNeeded();
      }

      FilteredRange(const FilteredRange&) = default;
      FilteredRange& operator=(const FilteredRange&) = default;
      FilteredRange(FilteredRange&&) = default;
      FilteredRange& operator=(FilteredRange&&) = default;

      auto& CurrentValue() const {
        return range_.CurrentValue();
      }

      void GoToBegin() {
        range_.GoToBegin();
        AdvanceToNextNonFilteredIfNeeded();
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

      bool Equals(const FilteredRange& o) const {
        return (range_.Equals(o.range_) && filter_ == o.filter_);
      }

      bool IsAtBegin() const {
        return range_.IsAtBegin();
      }

      bool IsAtEnd() const {
        return range_.IsAtEnd();
      }

     private:
      // Will not advance if current element is not filtered.
      void AdvanceToNextNonFilteredIfNeeded() {
        while (!IsAtEnd() && !filter_(range_.CurrentValue())) {
          range_.Advance();
          if (range_.IsAtEnd()) {
            break;
          }
        }
      }

      void RetreatToPreviousNonFilteredIfNeeded() {
        while (IsAtBegin() && !filter_(range_.CurrentValue())) {
          range_.Retreat();
          if (range_.IsAtBegin()) {
            break;
          }
        }
      }

      Range range_;
      Filter filter_;
    };

    template <typename Range>
    struct SimpleRangeWrapper {
      SimpleRangeWrapper(Range range)
          : range_(std::move(range)) {}

      SimpleRangeWrapper(const SimpleRangeWrapper&) = default;
      SimpleRangeWrapper& operator=(const SimpleRangeWrapper&) = default;
      SimpleRangeWrapper(SimpleRangeWrapper&&) = default;
      SimpleRangeWrapper& operator=(SimpleRangeWrapper&&) = default;

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
      ByValueTransformerRange(Range range, Transformer transformer)
          : SimpleRangeWrapper<Range>(std::move(range)),
            transformer_(std::move(transformer)) {}

      ByValueTransformerRange(const ByValueTransformerRange&) = default;
      ByValueTransformerRange& operator=(const ByValueTransformerRange&) = default;
      ByValueTransformerRange(ByValueTransformerRange&&) = default;
      ByValueTransformerRange& operator=(ByValueTransformerRange&&) = default;

      auto CurrentValue() const {
        return transformer_(this->range_.CurrentValue());
      }

      bool Equals(const ByValueTransformerRange& o) const {
        return (this->range_.Equals(o.range_) && transformer_ == o.transformer_);
      }

     private:
      Transformer transformer_;
    };

    // Transformer range.
    template <typename Range, typename Transformer>
    struct ByRefTransformerRange : SimpleRangeWrapper<Range> {
      ByRefTransformerRange(Range range, Transformer transformer)
          : SimpleRangeWrapper<Range>(std::move(range)),
            transformer_(std::move(transformer)) {}

      ByRefTransformerRange(const ByRefTransformerRange&) = default;
      ByRefTransformerRange& operator=(const ByRefTransformerRange&) = default;
      ByRefTransformerRange(ByRefTransformerRange&&) = default;
      ByRefTransformerRange& operator=(ByRefTransformerRange&&) = default;

      decltype(auto) CurrentValue() const {
        return transformer_(this->range_.CurrentValue());
      }

      bool Equals(const ByRefTransformerRange& o) const {
        return (this->range_.Equals(o.range_) && transformer_ == o.transformer_);
      }

     private:
      Transformer transformer_;
    };

    template <typename Range>
    struct RangeIterator {
      RangeIterator(Range range, bool is_end)
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
        return range_.Equals(o.range_);
      }

      bool operator!=(const RangeIterator& o) const {
        return !(*this == o);
      }

     private:
      Range range_;
    };

    template <typename Range>
    struct LazyWrapper {
      LazyWrapper(Range range)
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

      auto Deref() {
        using Entry = decltype(std::declval<Range>().CurrentValue());
        auto transformer = [](const Entry& entry) -> auto& { return *entry; };
        using InnerRange = ByRefTransformerRange<Range, decltype(transformer)>;
        return LazyWrapper<InnerRange>(
            InnerRange(range_, std::move(transformer)));
      }

      // TODO: Reverse()
      // TODO: AddressOf()
      // TODO: OrderBy()
      // TODO: SkipRepeating()

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

  template <typename Container>
  auto From(Container&& container) {
    // TODO: provide version to take container&& and store it inside Range.
    return From(container.begin(), container.end());
  }
}

#endif  //LAZY_CONTAINERS_LIBRARY
