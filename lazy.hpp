#ifndef LAZY_CONTAINERS_LIBRARY
#define LAZY_CONTAINERS_LIBRARY

#include <algorithm>
#include <exception>
#include <iterator>
#include <memory>
#include <type_traits>

#ifdef LAZY_DISABLE_RUNTIME_ASSERT
#  define LAZY_ASSERT(x)
#else
#  define LAZY_STRINGIZE_DETAIL(x) #x
#  define LAZY_STRINGIZE(x) LAZY_STRINGIZE_DETAIL(x)
#  define LAZY_ASSERT(x)                                        \
  if (!(x)) {                                                   \
    ::lazy::internal::DieDebugHook();                           \
    throw std::runtime_error(                                   \
        "[" __FILE__ ":" LAZY_STRINGIZE(__LINE__) "] " #x);     \
  }
#endif

// TODO:
// - Allow forward (i.e: non-backward) iterators
// - Add many more LAZY_ASSERTs
namespace lazy {
  namespace internal {
    void DieDebugHook() {}

    template <typename Container>
    using IteratorOf = typename Container::iterator;

    template <typename Range>
    using ReferenceType = decltype(*std::declval<typename Range::iterator>());

    template <typename Range>
    using ValueType = typename std::remove_reference<ReferenceType<Range>>::type;

    template <typename T>
    constexpr bool IsAssignable() {
      return std::is_copy_assignable<T>::value;
    }

    template <typename...>
    using void_t = void;

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

      AssignableFunctor(AssignableFunctor&&) = default;
      AssignableFunctor& operator=(AssignableFunctor&& o) = default;

      Functor functor;
    };

    template <typename Functor>
    struct AssignableFunctor<
        Functor,
        typename std::enable_if<!IsAssignable<Functor>()>::type> {
      AssignableFunctor(Functor functor_arg)
        : functor(std::move(functor_arg)) {}

      AssignableFunctor(AssignableFunctor&&) = default;
      AssignableFunctor& operator=(AssignableFunctor&& o) {
        functor.~Functor();
        new (&functor) Functor(std::move(o.functor));
        return *this;
      }

      Functor functor;
    };

    template <typename Iterator>
    struct SimpleRange {
      using iterator = Iterator;

      explicit SimpleRange(Iterator begin, Iterator end)
        : begin_(std::move(begin)),
          end_(std::move(end)) {}

      SimpleRange(SimpleRange&&) = default;
      SimpleRange& operator=(SimpleRange&&) = default;

      bool operator==(const SimpleRange& o) const {
        return (begin_ == o.begin_ && end_ == o.end_);
      }

      iterator begin() { return begin_; }
      iterator end() { return end_; }

     private:
      iterator begin_;
      iterator end_;
    };

    template <typename Container>
    struct ContainerOwner {
      explicit ContainerOwner(Container&& container)
        : container_(std::move(container)) {}

      ContainerOwner(ContainerOwner&&) = default;
      ContainerOwner& operator=(ContainerOwner&&) = default;

     protected:
      Container container_;
    };

    // Like SimpleRange, but also owns the container. Built for rvalues.
    template <typename Container>
    struct SimpleRangeOwner : ContainerOwner<Container>,
                              SimpleRange<typename Container::iterator> {
      explicit SimpleRangeOwner(Container&& container)
        : ContainerOwner<Container>(std::move(container)),
          SimpleRange<IteratorOf<Container>>(
              this->container_.begin(),
              this->container_.end()) {}

      SimpleRangeOwner(SimpleRangeOwner&& o) = default;
      SimpleRangeOwner& operator=(SimpleRangeOwner&& o) = default;
    };

    // Filtered range.
    template <typename Range, typename Filter>
    struct FilteredRange {
      explicit FilteredRange(Range range, Filter filter)
        : range_(std::move(range)),
          filter_(std::move(filter)) {}

      FilteredRange(FilteredRange&&) = default;
      FilteredRange& operator=(FilteredRange&&) = default;

      struct iterator {
        // iterator typedefs.
        using iterator_category = typename Range::iterator::iterator_category;
        using value_type = typename Range::iterator::value_type;
        using difference_type = typename Range::iterator::difference_type;
        using pointer = typename Range::iterator::pointer;
        using reference = typename Range::iterator::reference;

        explicit iterator(FilteredRange* const range,
                          typename Range::iterator iterator)
          : range_(range),
            iterator_(iterator) {
          this->AdvanceToNextNonFilteredIfNeeded();
        }

        iterator(const iterator&) = default;
        iterator& operator=(const iterator&) = default;
        iterator(iterator&&) = default;
        iterator& operator=(iterator&&) = default;

        decltype(auto) operator*() const {
          return *iterator_;
        }

        iterator& operator++() {
          ++iterator_;
          AdvanceToNextNonFilteredIfNeeded();
          return *this;
        }

        iterator& operator--() {
          --iterator_;
          RetreatToPreviousNonFilteredIfNeeded();
          return *this;
        }

        bool operator==(const iterator& o) const {
          return (range_ == o.range_ && iterator_ == o.iterator_);
        }

        bool operator!=(const iterator& o) const {
          return !(*this == o);
        }

       private:
        // Will not advance if current element is not filtered.
        void AdvanceToNextNonFilteredIfNeeded() {
          while (iterator_ != range_->range_.end() &&
                 !range_->filter_.functor(*iterator_)) {
            ++iterator_;
          }
        }

        void RetreatToPreviousNonFilteredIfNeeded() {
          while (iterator_ != range_->range_.begin() &&
                 !range_->filter_.functor(*iterator_)) {
            --iterator_;
          }
        }

        FilteredRange* const range_;
        typename Range::iterator iterator_;
      };

      bool operator==(const FilteredRange& o) const {
        return (range_ == o.range_ && filter_.functor == o.filter_.functor);
      }

      iterator begin() { return iterator(this, range_.begin()); }
      iterator end() { return iterator(this, range_.end()); }

     private:
      Range range_;
      AssignableFunctor<Filter> filter_;
    };

    template <typename Iterator>
    struct SimpleRangeIterator {
      // iterator typedefs.
      using iterator_category = typename Iterator::iterator_category;
      using value_type = typename Iterator::value_type;
      using difference_type = typename Iterator::difference_type;
      using pointer = typename Iterator::pointer;
      using reference = typename Iterator::reference;

      explicit SimpleRangeIterator(Iterator iterator)
        : iterator_(iterator) {}

      SimpleRangeIterator(const SimpleRangeIterator&) = default;
      SimpleRangeIterator& operator=(const SimpleRangeIterator&) = default;
      SimpleRangeIterator(SimpleRangeIterator&&) = default;
      SimpleRangeIterator& operator=(SimpleRangeIterator&&) = default;

      SimpleRangeIterator& operator++() {
        ++iterator_;
        return *this;
      }

      SimpleRangeIterator& operator--() {
        --iterator_;
        return *this;
      }

     protected:
      Iterator iterator_;
    };

    template <typename Range, typename Transformer>
    struct ByValueTransformerRange {
      explicit ByValueTransformerRange(Range range, Transformer transformer)
        : range_(std::move(range)),
          transformer_(std::move(transformer)) {}

      ByValueTransformerRange(ByValueTransformerRange&&) = default;
      ByValueTransformerRange& operator=(ByValueTransformerRange&&) = default;

      struct iterator : SimpleRangeIterator<typename Range::iterator> {
        iterator(const ByValueTransformerRange* const range,
                 typename Range::iterator iterator)
          : SimpleRangeIterator<typename Range::iterator>(std::move(iterator)),
            range_(range) {}

        iterator(const iterator&) = default;
        iterator& operator=(const iterator&) = default;
        iterator(iterator&&) = default;
        iterator& operator=(iterator&&) = default;

        auto operator*() const {
          return range_->transformer_.functor(*this->iterator_);
        }

        bool operator==(const iterator& o) const {
          return (range_ == o.range_ && this->iterator_ == o.iterator_);
        }

        bool operator!=(const iterator& o) const {
          return !(*this == o);
        }

       private:
        const ByValueTransformerRange* const range_;
      };

      bool operator==(const ByValueTransformerRange& o) const {
        return (this->range_ == o.range_ &&
                transformer_.functor == o.transformer_.functor);
      }

      iterator begin() { return iterator(this, range_.begin()); }
      iterator end() { return iterator(this, range_.end()); }

     private:
      Range range_;
      AssignableFunctor<Transformer> transformer_;
      friend struct iterator;
    };

    // Transformer range.
    template <typename Range, typename Transformer>
    struct ByRefTransformerRange {
      explicit ByRefTransformerRange(Range range, Transformer transformer)
        : range_(std::move(range)),
          transformer_(std::move(transformer)) {}

      ByRefTransformerRange(ByRefTransformerRange&&) = default;
      ByRefTransformerRange& operator=(ByRefTransformerRange&&) = default;

      struct iterator : SimpleRangeIterator<typename Range::iterator> {
        iterator(const ByRefTransformerRange* const range,
                 typename Range::iterator iterator)
          : SimpleRangeIterator<typename Range::iterator>(std::move(iterator)),
            range_(range) {}

        iterator(const iterator&) = default;
        iterator& operator=(const iterator&) = default;
        iterator(iterator&&) = default;
        iterator& operator=(iterator&&) = default;

        decltype(auto) operator*() const {
          return range_->transformer_.functor(*this->iterator_);
        }

        bool operator==(const iterator& o) const {
          return (this->range_ == o.range_ && this->iterator_ == o.iterator_);
        }

        bool operator!=(const iterator& o) const {
          return !(*this == o);
        }

       private:
        const ByRefTransformerRange* const range_;
      };

      bool operator==(const ByRefTransformerRange& o) const {
        return (this->range_ == o.range_ &&
                transformer_.functor == o.transformer_.functor);
      }

      iterator begin() { return iterator(this, range_.begin()); }
      iterator end() { return iterator(this, range_.end()); }

     private:
      Range range_;
      AssignableFunctor<Transformer> transformer_;
      friend struct iterator;
    };

    template <typename T>
    struct DereferenceFunctor {
      DereferenceFunctor() = default;

      DereferenceFunctor(const DereferenceFunctor&) = default;
      DereferenceFunctor(DereferenceFunctor&&) = default;
      DereferenceFunctor& operator=(const DereferenceFunctor&) = default;
      DereferenceFunctor& operator=(DereferenceFunctor&&) = default;

      decltype(auto) operator()(const T& t) const {
        return *t;
      }

      bool operator==(const DereferenceFunctor& o) const {
        return true;
      }
    };

    template <typename Range>
    struct DereferenceRange
        : ByRefTransformerRange<Range,
                                DereferenceFunctor<ReferenceType<Range>>> {
      explicit DereferenceRange(Range range)
        : ByRefTransformerRange<Range,
                                DereferenceFunctor<ReferenceType<Range>>>(
              std::move(range), DereferenceFunctor<ReferenceType<Range>>()) {}

      DereferenceRange(DereferenceRange&&) = default;
      DereferenceRange& operator=(DereferenceRange&&) = default;
    };

    template <typename Range>
    struct ReverseRange {
      explicit ReverseRange(Range range)
        : range_(std::move(range)) {}

      ReverseRange(ReverseRange&&) = default;
      ReverseRange& operator=(ReverseRange&&) = default;

      struct iterator {
        // iterator typedefs.
        using iterator_category = typename Range::iterator::iterator_category;
        using value_type = typename Range::iterator::value_type;
        using difference_type = typename Range::iterator::difference_type;
        using pointer = typename Range::iterator::pointer;
        using reference = typename Range::iterator::reference;

        explicit iterator(ReverseRange* const range,
                          typename Range::iterator iterator,
                          bool is_at_rend = false)
          : range_(range),
            iterator_(iterator),
            is_at_rend_(is_at_rend) {}

        iterator(const iterator&) = default;
        iterator& operator=(const iterator&) = default;
        iterator(iterator&&) = default;
        iterator& operator=(iterator&&) = default;

        decltype(auto) operator*() const {
          LAZY_ASSERT(iterator_ != range_->range_.end());
          LAZY_ASSERT(!is_at_rend_);
          return *iterator_;
        }

        iterator& operator++() {
          LAZY_ASSERT(iterator_ != range_->range_.end());
          LAZY_ASSERT(!is_at_rend_);
          if (iterator_ == range_->range_.begin()) {
            is_at_rend_ = true;
          } else {
            --iterator_;
          }
          return *this;
        }

        iterator& operator--() {
          LAZY_ASSERT(iterator_ != range_->range_.end());
          if (is_at_rend_) {
            LAZY_ASSERT(iterator_ == range_->range_.begin());
            is_at_rend_ = false;
          } else {
            ++iterator_;
            LAZY_ASSERT(iterator_ != range_->range_.begin());
          }
          return *this;
        }

        bool operator==(const iterator& o) const {
          return (range_ == o.range_ &&
                  iterator_ == o.iterator_ &&
                  is_at_rend_ == o.is_at_rend_);
        }

        bool operator!=(const iterator& o) const {
          return !(*this == o);
        }

       private:
        ReverseRange* const range_;
        typename Range::iterator iterator_;
        bool is_at_rend_;
      };

      bool operator==(const ReverseRange& o) const {
        return (range_ == o.range_);
      }

      iterator begin() {
        auto inner_it = range_.end();
        if (inner_it == range_.begin()) {
          return iterator(this, inner_it, true);
        } else {
          --inner_it;
          return iterator(this, inner_it);
        }
      }

      iterator end() {
        return iterator(this, range_.begin(), true);
      }

     private:
      Range range_;
    };

    // LazyWrapper wraps a Range with functions that allow manipulating it, such
    // as Where(), Reverse(), etc.
    // It is only allowed to be used in telescoping (like:
    // From(x).Where().Sort()), and thus all methods only exist for rvalues.
    template <typename Range>
    struct LazyWrapper {
      explicit LazyWrapper(Range range)
        : range_(std::move(range)) {}

      LazyWrapper(LazyWrapper&&) = default;
      LazyWrapper& operator=(LazyWrapper&&) = default;

      template <typename Filter>
      auto Where(Filter filter) && {
        using InnerRange = FilteredRange<Range, Filter>;
        return LazyWrapper<InnerRange>(InnerRange(std::move(range_),
                                                  std::move(filter)));
      }

      template <typename Transformer>
      auto Transform(Transformer transformer) && {
        using InnerRange = ByValueTransformerRange<Range, Transformer>;
        return LazyWrapper<InnerRange>(
            InnerRange(std::move(range_), std::move(transformer)));
      }

      auto Keys() && {
        auto transformer = [](const ValueType<Range>& entry) {
          return entry.first;
        };
        using InnerRange = ByValueTransformerRange<Range, decltype(transformer)>;
        return LazyWrapper<InnerRange>(
            InnerRange(std::move(range_), std::move(transformer)));
      }

      auto Values() && {
        auto transformer = [](ValueType<Range>& entry) -> auto& {
          return entry.second;
        };
        using InnerRange = ByRefTransformerRange<Range, decltype(transformer)>;
        return LazyWrapper<InnerRange>(
            InnerRange(std::move(range_), std::move(transformer)));
      }

      auto Dereference() && {
        using InnerRange = DereferenceRange<Range>;
        return LazyWrapper<InnerRange>(InnerRange(std::move(range_)));
      }

      auto AddressOf() && {
        auto transformer = [](ValueType<Range>& entry) { return &entry; };
        using InnerRange = ByValueTransformerRange<Range, decltype(transformer)>;
        return LazyWrapper<InnerRange>(
            InnerRange(std::move(range_), std::move(transformer)));
      }

      auto Reverse() && {
        using InnerRange = ReverseRange<Range>;
        return LazyWrapper<InnerRange>(InnerRange(std::move(range_)));
      }

      // Iterates over the range in a sorted fashion, while returning a
      // reference to each of the values of the original list. You may modify
      // values unless otherwise limited.
      // TODO: This does not yet occur lazily.
      auto Sort() && {
        return std::move(*this).Sort(std::less<ValueType<Range>>());
      }
      template <typename Comparator>
      auto Sort(Comparator comparator) && {
        using Pointer = ValueType<Range>*;
        using TmpVector = std::vector<Pointer>;
        TmpVector v;
        for (auto& value : *this) {
          v.push_back(&value);
        }
        std::sort(v.begin(), v.end(), [&](Pointer a, Pointer b) {
          return comparator(*a, *b);
        });

        // Owns both sorted and unsorted ranges.
        struct OwnerRange : internal::SimpleRangeOwner<TmpVector> {
          OwnerRange(TmpVector&& sorted, Range range)
            : internal::SimpleRangeOwner<TmpVector>(std::move(sorted)),
              range_(std::move(range)) {}

          OwnerRange(OwnerRange&&) = default;
          OwnerRange& operator=(OwnerRange&&) = default;

         protected:
          Range range_;
        };

        using DerefRange = DereferenceRange<OwnerRange>;
        return LazyWrapper<DerefRange>(DerefRange(OwnerRange(
            std::move(v), std::move(range_))));
      }

      // TODO: SkipRepeating()
      // TODO: SortUnique()

      auto begin() { return range_.begin(); }
      auto end() { return range_.end(); }

      // Implicit cast to any container.
      // TODO: use std::move() if we own the container(?)
      template <typename Container>
      operator Container() && {
        Container container;
        auto output_it = std::inserter(container, container.end());
        for (const auto& it : *this) {
          *output_it = it;
          ++output_it;
        }
        return container;
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
