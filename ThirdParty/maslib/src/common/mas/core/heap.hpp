#ifndef MAS_CORE_HEAP_HPP_
#define MAS_CORE_HEAP_HPP_

// for parellel make_heap implementation
#if __cplusplus >= 201103L
#include <vector>
#include <atomic>
#include "mas/concurrency/thread.h"
#endif

namespace mas {
namespace heap {

//===========================================================================
// STD-COMPATIBLE VERSIONS
//===========================================================================

#if __cplusplus >= 201103L
#define __MAS_MOVE(__a) std::move(__a)
#else
#define __MAS_MOVE(__a) (__a)
#endif

/**
 * Bubble-up operation (internal use only)
 * @param first Start of heap
 * @param length End of heap
 * @param pos position in heap to potentially move up
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator, typename SizeType, typename ValueType,
        typename Compare>
void __up_heap(RandomAccessIterator first, SizeType pos, ValueType p,
        Compare& compare) {

    SizeType parent = (pos - 1) / 2;
    SizeType child = pos;
    while (child > 0 && compare(*(first + parent), p)) {

        *(first + child) = __MAS_MOVE(*(first + parent));
        child = parent;
        parent = (child - 1) / 2;
    }

    // move value into new slot
    *(first + child) = __MAS_MOVE(p);

}

/**
 * Bubble-up operation (internal use only)
 * @param first Start of heap
 * @param length End of heap
 * @param pos position in heap to potentially move up
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator, typename SizeType, typename ValueType>
void __up_heap(RandomAccessIterator first, SizeType pos, ValueType p) {

    SizeType parent = (pos - 1) / 2;
    SizeType child = pos;
    while (child > 0 && (*(first + parent) < p)) {

        *(first + child) = __MAS_MOVE(*(first + parent));
        child = parent;
        parent = (child - 1) / 2;
    }

    // move value into new slot
    *(first + child) = __MAS_MOVE(p);

}

/**
 * Bubble-down operation (internal use only)
 * @param first Start of heap
 * @param length End of heap
 * @param hole empty position in heap to potentially move down
 * @param p value currently popped out of the hole
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator, typename SizeType, typename ValueType,
        typename Compare>
void __down_heap(RandomAccessIterator first, SizeType length, SizeType hole,
        ValueType p, Compare& compare) {

    SizeType parent = hole;
    SizeType child = 2 * parent + 1;   // first child

    while (child < length) {

        // check if second child is larger
        if ((child < length - 1)
                && compare(*(first + child), *(first + child + 1))) {
            child++;  // second child
        }

        // compare parent with child
        if (compare(*(first + child), p)) {
            // parent is greater, we are done
            break;
        }

        // replace hole with child
        *(first + parent) = __MAS_MOVE(*(first + child));

        // move down next branch
        parent = child;
        child = 2 * parent + 1;
    }

    // move value into new slot
    *(first + parent) = __MAS_MOVE(p);

}

/**
 * Bubble-down operation (internal use only)
 * @param first Start of heap
 * @param length End of heap
 * @param hole empty position in heap to potentially move down
 * @param p value currently popped out of the hole
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator, typename SizeType, typename ValueType>
void __down_heap(RandomAccessIterator first, SizeType length, SizeType hole,
        ValueType p) {

    SizeType parent = hole;
    SizeType child = 2 * parent + 1;     // first child
    while (child < length) {

        // check if second child is larger
        if ((child < length - 1) && (*(first + child) < *(first + child + 1))) {
            child++;  // second child
        }

        // compare parent with child
        if (*(first + child) < p) {
            // parent is greater, we are done
            break;
        }

        // replace hole with child
        *(first + parent) = __MAS_MOVE(*(first + child));

        // move down next branch
        parent = child;
        child = 2 * parent + 1;
    }

    // move value into new slot
    *(first + parent) = __MAS_MOVE(p);

}

template<typename RandomAccessIterator, typename Compare>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator result, Compare& compare) {
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    ValueType value = __MAS_MOVE(*last); // back gets moved to the front, since reducing length
    *result = __MAS_MOVE(*first);
    mas::heap::__down_heap(first, last - first, SizeType(0), __MAS_MOVE(value),
            compare);

}

template<typename RandomAccessIterator>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator result) {
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    ValueType value = __MAS_MOVE(*last); //back gets moved to front, since reducing length
    *result = __MAS_MOVE(*first);
    mas::heap::__down_heap(first, last - first, SizeType(0), __MAS_MOVE(value));
}

/**
 * @brief  Constructs a max binary heap over the given range using a comparison
 * functor.
 * @param  first  start of heap
 * @param  last   end of heap
 * @param  compare  comparison functor, compare(a,b)=true if a < b
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator, typename Compare>
void make_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare) {
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;
    // starting with deepest parent, bubble down
    const SizeType len = last - first;
    for (SizeType parent = len / 2; parent-- > 0;) {
        __down_heap(first, len, parent, __MAS_MOVE(*(first + parent)), compare);
    }
}

/**
 * @brief  Constructs a max binary heap over the given range.
 * @param  first  start of heap
 * @param  last   end of heap
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator>
void make_heap(RandomAccessIterator first, RandomAccessIterator last) {
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;
    // starting with deepest parent, bubble down
    const SizeType len = last - first;
    for (SizeType parent = len / 2; parent-- > 0;) {
        __down_heap(first, len, parent, __MAS_MOVE(*(first + parent)));
    }
}

/**
 * @brief Extends a max binary heap [first, last-1) to [first, last) by
 * placing the value at (last-1) into its proper location in the heap.
 * @param first start of heap
 * @param last end of heap
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator, typename Compare>
void push_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    // bubble up if parent is smaller
    SizeType lastPos = last - first - 1; // length-1
    SizeType parent = (lastPos - 1) / 2;
    if ((lastPos > 0) && compare(*(first + parent), *(last - 1))) {
        __up_heap(first, lastPos, __MAS_MOVE(*(first + lastPos)), compare);
    }
}

/**
 * @brief Extends a max binary heap [first, last-1) to [first, last) by
 * placing the value at (last-1) into its proper location in the heap.
 * @param first start of heap
 * @param last end of heap
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator>
void push_heap(RandomAccessIterator first, RandomAccessIterator last) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    // bubble up if parent is smaller
    SizeType lastPos = last - first - 1; // length-1
    SizeType parent = (lastPos - 1) / 2;
    if ((lastPos > 0) && (*(first + parent) < *(last - 1))) {
        __up_heap(first, lastPos, __MAS_MOVE(*(first + lastPos)));
    }
}

/**
 * @brief Removes the largest element in the heap, moving it to the
 * location (last-1)
 * @param first start of heap
 * @param last end of heap
 * @param compare comparison functor, compare(a,b)=true if a < b
 */
template<typename RandomAccessIterator, typename Compare>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare) {

    if (last - first > 1) {
        last--;
        mas::heap::__pop_heap(first, last, last, compare);
    }
}

/**
 * @brief Removes the largest element in the heap, moving it to the
 * location (last-1)
 * @param first start of heap
 * @param last end of heap
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last) {

    if (last - first > 1) {
        last--;
        mas::heap::__pop_heap(first, last, last);
    }
}

/**
 * @brief Sorts the range [first, last) in ascending order, given
 * that it currently represents a binary heap.
 * @param first start of heap
 * @param last end of heap
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator, typename Compare>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare) {
    while (last - first > 1) {
        last--;
        mas::heap::__pop_heap(first, last, last, compare);
    }
}

/**
 * @brief Sorts the range [first, last) in ascending order, given
 * that it currently represents a binary heap.
 * @param first start of heap
 * @param last end of heap
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last) {
    while (last - first > 1) {
        last--;
        mas::heap::__pop_heap(first, last, last);
    }
}

/**
 * @brief Finds the first element in [first, last) that is not in the
 * correct order assuming a max binary heap.
 * @param first start of heap
 * @param last end of heap
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @return the first invalid iterator, or last if the heap is valid
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator, typename Compare>
RandomAccessIterator is_heap_until(RandomAccessIterator first,
        RandomAccessIterator last, Compare compare) {

    RandomAccessIterator parent = first;
    for (RandomAccessIterator child = first + 1; child < last; child++) {
        // first child
        if (compare(*parent, *child)) {
            return child;
        }
        // second child
        child++;
        if (child < last && compare(*parent, *child)) {
            return child;
        }
        // move to next parent
        parent++;
    }
    return last;
}

/**
 * @brief Finds the first element in [first, last) that is not in the
 * correct order assuming a max binary heap.
 * @param first start of heap
 * @param last end of heap
 * @return the first invalid iterator, or last if the heap is valid
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator>
RandomAccessIterator is_heap_until(RandomAccessIterator first,
        RandomAccessIterator last) {

    RandomAccessIterator parent = first;
    for (RandomAccessIterator child = first + 1; child < last; child++) {
        // first child
        if ((*parent < *child)) {
            return child;
        }
        // second child
        child++;
        if (child < last && (*parent < *child)) {
            return child;
        }
        // move to next parent
        parent++;
    }
    return last;
}

/**
 * @brief Determines if the range [first, last) represents a valid
 * max binary heap.
 * @param first start of heap
 * @param last end of heap
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @return true if range represents a valid max binary heap
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator, typename Compare>
inline bool is_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare) {
    return (mas::heap::is_heap_until(first, last, compare) == last);
}

/**
 * @brief Determines if the range [first, last) represents a valid
 * max binary heap.
 * @param first start of heap
 * @param last end of heap
 * @return true if range represents a valid max binary heap
 * @ingroup std::heap_algorithms
 */
template<typename RandomAccessIterator>
inline bool is_heap(RandomAccessIterator first, RandomAccessIterator last) {
    return (mas::heap::is_heap_until(first, last) == last);
}

//========================================================================
// Non-standard additions
//========================================================================

template<typename RandomAccessIterator, typename Compare>
void __pop_heap_pos(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator pos, RandomAccessIterator result,
        Compare& compare) {
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    ValueType value = __MAS_MOVE(*result); // move out element from last position (will be re-inserted)
    *result = __MAS_MOVE(*pos);             // move value from 'pos' to the end
    SizeType hole = pos - first;          // location of the hole

    SizeType parent = (hole - 1) / 2;

    // see if we need to move value up
    if (pos > first && compare(*(first + parent), value)) {
        // value needs to move up
        __up_heap(first, hole, __MAS_MOVE(value), compare);
    } else {
        // move down or stay in place
        __down_heap(first, last - first, hole, value, compare);
    }

}

template<typename RandomAccessIterator>
void __pop_heap_pos(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator pos, RandomAccessIterator result) {
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    ValueType value = __MAS_MOVE(*result); // move out element from last position (will be re-inserted)
    *result = __MAS_MOVE(*pos);             // move value from 'pos' to the end
    SizeType hole = pos - first;          // location of the hole

    SizeType parent = (hole - 1) / 2;

    // see if we need to move value up
    if (pos > first && (*(first + parent) < value)) {
        // value needs to move up
        __up_heap(first, hole, __MAS_MOVE(value));
    } else {
        // move down or stay in place
        __down_heap(first, last - first, hole, value);
    }

}

/**
 * Updates a max binary heap for a single element whose comparison
 * value has changed
 * @param first start of heap
 * @param last end of heap
 * @param pos element whose position requires updating
 * @param compare comparison functor, compare(a,b)=true if a < b
 */
template<typename RandomAccessIterator, typename Compare>
void update_heap(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator pos, Compare compare) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    SizeType posd = pos - first;
    // check if parent is smaller or child is bigger, bubble where necessary
    if ((pos > first) && compare(*(first + (posd - 1) / 2), *(first + posd))) {
        __up_heap(first, posd, __MAS_MOVE(*(first + posd)), compare);
    } else {
        __down_heap(first, last - first, posd, __MAS_MOVE(*(first + posd)),
                compare);
    }

}

/**
 * Updates a max binary heap for a single element whose comparison
 * value has changed
 * @param first start of heap
 * @param last end of heap
 * @param pos element whose position requires updating
 */
template<typename RandomAccessIterator>
void update_heap(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator pos) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    SizeType posd = pos - first;
    // check if parent is smaller or child is bigger, bubble where necessary
    if ((pos > first) && (*(first + (posd - 1) / 2)) < *(first + posd)) {
        __up_heap(first, posd, __MAS_MOVE(*(first + posd)));
    } else {
        __down_heap(first, last - first, posd, __MAS_MOVE(*(first + posd)));
    }

}

/**
 * @brief Removes the element at iterator location pos from the heap, moving
 * it to the location (last-1)
 * @param first start of heap
 * @param last end of heap
 * @param pos position to move
 * @param compare comparison functor, compare(a,b)=true if a < b
 */
template<typename RandomAccessIterator, typename Compare>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator pos, Compare compare) {

    // if not at the end, pop it out
    if (last - pos > 1) {
        last--;
        mas::heap::__pop_heap_pos(first, last, pos, last, compare);
    }
}

/**
 * @brief Removes the element at iterator location pos from the heap, moving
 * it to the location (last-1)
 * @param first start of heap
 * @param last end of heap
 * @param pos position to move
 */
template<typename RandomAccessIterator>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator pos) {

    // if not at the end, pop it out
    if (last - pos > 1) {
        last--;
        mas::heap::__pop_heap_pos(first, last, pos, last);
    }
}

//===========================================================================
// CALLBACK VERSIONS
//===========================================================================

/**
 * Bubble-up operation (internal use only)
 * @param first Start of heap
 * @param length End of heap
 * @param pos position in heap to potentially move up
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @param moved callback, moved(val&, const a, const b) is called after val is moved from first+a to first+b
 */
template<typename RandomAccessIterator, typename SizeType, typename ValueType,
        typename Compare, typename MoveCallback>
void __up_heap(RandomAccessIterator first, SizeType pos, ValueType p,
        SizeType ppos, Compare& compare, MoveCallback& moved) {

    SizeType parent = (pos - 1) / 2;
    SizeType child = pos;
    while (child > 0 && compare(*(first + parent), p)) {

        *(first + child) = __MAS_MOVE(*(first + parent));
        moved(*(first + child), parent, child);
        child = parent;
        parent = (child - 1) / 2;
    }

    // move value into new slot
    *(first + child) = __MAS_MOVE(p);
    moved(*(first + child), ppos, child);

}

/**
 * Bubble-down operation (internal use only)
 * @param first Start of heap
 * @param length End of heap
 * @param hole empty position in heap to potentially move down
 * @param p value currently popped out of the hole
 * @param ppos original position of value p
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @param moved callback, moved(val&, const a, const b) is called after val is moved from first+a to first+b
 */
template<typename RandomAccessIterator, typename SizeType, typename ValueType,
        typename Compare, typename MoveCallback>
void __down_heap(RandomAccessIterator first, SizeType length, SizeType hole,
        ValueType p, SizeType ppos, Compare& compare, MoveCallback& moved) {

    SizeType parent = hole;
    SizeType child = 2 * parent + 1;   // first child

    while (child < length) {

        // check if second child is larger
        if ((child < length - 1)
                && compare(*(first + child), *(first + child + 1))) {
            child++;  // second child
        }

        // compare parent with child
        if (compare(*(first + child), p)) {
            // parent is greater, we are done
            break;
        }

        // replace hole with child
        *(first + parent) = __MAS_MOVE(*(first + child));
        moved(*(first + parent), child, parent);

        // move down next branch
        parent = child;
        child = 2 * parent + 1;
    }

    // move value into new slot
    *(first + parent) = __MAS_MOVE(p);
    moved(*(first + parent), ppos, parent);

}

template<typename RandomAccessIterator, typename Compare, typename MoveCallback>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator result, Compare& compare, MoveCallback& moved) {
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    ValueType value = __MAS_MOVE(*last);  // move out last (making heap smaller)
    *result = __MAS_MOVE(*first);
    SizeType ppos = last - first;
    mas::heap::__down_heap(first, ppos, SizeType(0), __MAS_MOVE(value), ppos,
            compare, moved);

}

template<typename RandomAccessIterator, typename Compare, typename MoveCallback>
void __pop_heap_pos(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator pos, RandomAccessIterator result, Compare& compare,
        MoveCallback& moved) {
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    ValueType value = __MAS_MOVE(*last); // move out element from last position (will be re-inserted)
    *result = __MAS_MOVE(*pos);           // pop out element at given position

    SizeType hole = pos - first;          // location of the hole
    SizeType parent = (hole - 1) / 2;       // location of parent

    // see if we need to move value up
    if (pos > first && compare(*(first + parent), value)) {
        // value needs to move up
        __up_heap(first, hole, __MAS_MOVE(value), last - first, compare, moved);
    } else {
        // move down or stay in place
        SizeType ppos = last - first; // position of element being moved
        __down_heap(first, ppos, hole, value, ppos, compare, moved);
    }

}

/**
 * @brief  Constructs a max binary heap over the given range using a comparison
 * functor.
 * @param  first  start of heap
 * @param  last   end of heap
 * @param  compare  comparison functor, compare(a,b)=true if a < b
 * @param moved callback, moved(val&, const a, const b) is called after val is moved from first+a to first+b
 */
template<typename RandomAccessIterator, typename Compare, typename MoveCallback>
void make_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare, MoveCallback moved) {
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;
    // starting with deepest parent, bubble down
    const SizeType len = last - first;
    for (SizeType parent = len / 2; parent-- > 0;) {
        __down_heap(first, len, parent, __MAS_MOVE(*(first + parent)), parent,
                compare, moved);
    }
}

/**
 * @brief Extends a max binary heap [first, last-1) to [first, last) by
 * placing the value at (last-1) into its proper location in the heap.
 * @param first start of heap
 * @param last end of heap
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @param moved callback, moved(val&, const a, const b) is called after val is moved from first+a to first+b
 */
template<typename RandomAccessIterator, typename Compare, typename MoveCallback>
void push_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare, MoveCallback moved) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    // bubble up if parent is smaller
    SizeType lastPos = last - first - 1; // length-1
    SizeType parent = (lastPos - 1) / 2;
    if ((lastPos > 0) && compare(*(first + parent), *(last - 1))) {
        __up_heap(first, lastPos, __MAS_MOVE(*(first + lastPos)), lastPos,
                compare, moved);
    } else {
        moved(*(first + lastPos), lastPos, lastPos);
    }

}

/**
 * @brief Removes the largest element in the heap, moving it to the
 * location (last-1)
 * @param first start of heap
 * @param last end of heap
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @param moved callback, moved(val&, const a, const b) is called after val is moved from first+a to first+b
 */
template<typename RandomAccessIterator, typename Compare, typename MoveCallback>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare, MoveCallback moved) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;
    RandomAccessIterator end = last;
    if (end - first > 1) {
        end--;
        mas::heap::__pop_heap(first, end, end, compare, moved);
    }
    moved(*(last - 1), SizeType(0), last - first - 1);
}

/**
 * @brief Removes the element at iterator location pos from the heap, moving
 * it to the location (last-1)
 * @param first start of heap
 * @param last end of heap
 * @param pos position to move
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @param moved callback, moved(val&, const a, const b) is called after val is moved from first+a to first+b
 */
template<typename RandomAccessIterator, typename Compare, typename MoveCallback>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator pos, Compare compare, MoveCallback moved) {

    // if not at the end, pop it out
    RandomAccessIterator end = last;
    if (end - pos > 1) {
        end--;
        mas::heap::__pop_heap_pos(first, end, pos, end, compare, moved);
    }
    moved(*(last - 1), pos - first, last - first - 1);
}

/**
 * @brief Sorts the range [first, last) in ascending order, given
 * that it currently represents a binary heap.
 * @param first start of heap
 * @param last end of heap
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @param moved callback, moved(val&, const a, const b) is called after val is moved from first+a to first+b
 */
template<typename RandomAccessIterator, typename Compare, typename MoveCallback>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare, MoveCallback moved) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;
    while (last - first > 1) {
        last--;
        mas::heap::__pop_heap(first, last, last, compare, moved);
        moved(*last, SizeType(0), last - first);
    }
}

/**
 * Updates a max binary heap for a single element whose comparison
 * value has changed
 * @param first start of heap
 * @param last end of heap
 * @param pos element whose position requires updating
 * @param compare comparison functor, compare(a,b)=true if a < b
 * @param moved callback, moved(val&, const a, const b) is called after val is moved from first+a to first+b
 */
template<typename RandomAccessIterator, typename Compare, typename MoveCallback>
void update_heap(RandomAccessIterator first, RandomAccessIterator last,
        RandomAccessIterator pos, Compare compare, MoveCallback moved) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    SizeType posd = pos - first;

    // check if parent is smaller or child is bigger, bubble where necessary
    if ((pos > first) && compare(*(first + (posd - 1) / 2), *(first + posd))) {
        __up_heap(first, posd, __MAS_MOVE(*(first + posd)), posd, compare,
                moved);
    } else {
        // might have to move down if less then either child
        __down_heap(first, last - first, posd, __MAS_MOVE(*(first + posd)),
                posd, compare, moved);
    }
}

//========================================================================
// Parallel versions
//========================================================================

#if __cplusplus >= 201103L

template<typename RandomAccessIterator, typename SizeType, typename Compare>
void __down_block(RandomAccessIterator first, SizeType length,
        SizeType blockFront, SizeType blockLength, Compare& compare) {
    SizeType parent = blockFront + blockLength - 1;
    for (SizeType i = 0; i < blockLength; i++) {
        __down_heap(first, length, parent, __MAS_MOVE(*(first + parent)),
                compare);
        parent--;
    }
}

template<typename RandomAccessIterator, typename SizeType>
void __down_block(RandomAccessIterator first, SizeType length,
        SizeType blockFront, SizeType blockLength) {
    SizeType parent = blockFront + blockLength - 1;
    for (SizeType i = 0; i < blockLength; i++) {
        __down_heap(first, length, parent, __MAS_MOVE(*(first + parent)));
        parent--;
    }
}

template<typename RandomAccessIterator, typename SizeType, typename Compare>
void __thread_worker(int threadIdx, int nthreads,
        std::atomic<size_t>& blocksRemaining, SizeType blockSize,
        mas::concurrency::rolling_barrier& barrier,
        RandomAccessIterator first, SizeType len, Compare& compare) {

    bool complete = false;
    while (!complete) {
        size_t processBlock = --blocksRemaining;

        // safe check for negative value accounting for overflow
        if (processBlock + nthreads < (size_t)nthreads) {
            complete = true;
            barrier.set(threadIdx, -1);
            break;
        }

        // we are processing block 'processBlock'
        SizeType blockFront = processBlock * blockSize;
        SizeType firstChild = 2 * blockFront + 1;

        // check that we can go
        barrier.set(threadIdx, len-blockFront-blockSize);
        SizeType waitFor = len - firstChild - 1;
        while (!barrier.poll(threadIdx, waitFor)) {
            std::this_thread::yield();
        }
        __down_block(first, len, blockFront, blockSize, compare);
        barrier.set(threadIdx, -1);
    }
}

template<typename RandomAccessIterator, typename SizeType>
void __thread_worker(int threadIdx, int nthreads,
        std::atomic<size_t>& blocksRemaining, SizeType blockSize,
        mas::concurrency::rolling_barrier& barrier,
        RandomAccessIterator first, SizeType len) {

    bool complete = false;
    while (!complete) {
        size_t processBlock = --blocksRemaining;

        // safe check for negative value accounting for overflow
        if (processBlock + nthreads < nthreads) {
            complete = true;
            barrier.set(threadIdx, -1);
            break;
        }

        // we are processing block 'processBlock'
        SizeType blockFront = processBlock * blockSize;
        SizeType firstChild = 2 * blockFront + 1;

        // check that we can go
        barrier.set(threadIdx, len-blockFront-blockSize);
        SizeType waitFor = len - firstChild - 1;
        while (!barrier.poll(threadIdx, waitFor)) {
            std::this_thread::yield();
        }
        __down_block(first, len, blockFront, blockSize);
        barrier.set(threadIdx, -1);
    }
}

template<typename RandomAccessIterator, typename Compare>
void parallel_make_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare,
        typename std::iterator_traits<RandomAccessIterator>::difference_type blockSize,
        int maxThreads) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    // starting with deepest parent, bubble down
    const SizeType len = last - first;
    SizeType processEnd = len / 2;

    // number of blocks
    size_t nblocks = processEnd / blockSize;
    if (processEnd % blockSize != 0) {
        nblocks++;
    }

    if (maxThreads <= 0) {
        maxThreads = std::max(std::thread::hardware_concurrency(), (unsigned int)2); // try 2 threads if hardware_concurrency returns 0
    }

    int nthreads = (int)(std::min(nblocks, (size_t)maxThreads));
    if (nthreads < 1) {
        nthreads = 1;
    }
    mas::concurrency::rolling_barrier barrier(nthreads);
    std::vector<std::thread> threads;
    threads.reserve(nthreads - 1);

    std::atomic<size_t> blocksRemaining;
    blocksRemaining.store(nblocks);

    // current thread's work
    {
        int threadIdx = nthreads - 1;  // this thread is the last one
        // current thread will start the last block to avoid having to check partial block
        size_t processBlock = --blocksRemaining;

        // front/back
        SizeType blockFront = processBlock * blockSize;
        SizeType blockLength = processEnd - blockFront;

        // initialize other threads
        for (int i = 0; i < nthreads - 1; i++) {
            threads.push_back(
                    std::thread(
                            __thread_worker<RandomAccessIterator, SizeType,
                                    Compare>, i, nthreads,
                            std::ref(blocksRemaining), blockSize,
                            std::ref(barrier), first, len,
                            std::ref(compare)));
        }

        mas::concurrency::thread_group<std::vector<std::thread>> threadGroup(threads); // for closing off threads

        // process current block
        __down_block(first, len, blockFront, blockLength, compare);
        barrier.set(threadIdx, -1);

        // continue processing other blocks
        __thread_worker(threadIdx, nthreads, blocksRemaining, blockSize,
                barrier, first, len, compare);

        // done!!
        // std::cout << "DONE!" << std::endl;

    }

}

template<typename RandomAccessIterator>
void parallel_make_heap(RandomAccessIterator first, RandomAccessIterator last,
        typename std::iterator_traits<RandomAccessIterator>::difference_type blockSize,
        int maxThreads) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    // starting with deepest parent, bubble down
    const SizeType len = last - first;
    SizeType processEnd = len / 2;

    // number of blocks
    size_t nblocks = processEnd / blockSize;
    if (processEnd % blockSize != 0) {
        nblocks++;
    }

    if (maxThreads <= 0) {
        maxThreads = std::max(std::thread::hardware_concurrency(), (unsigned int)2); // try 2 threads if hardware_concurrency returns 0
    }

    size_t nthreads = std::min(nblocks, (size_t)maxThreads);
    mas::concurrency::rolling_barrier barrier(nthreads);
    std::vector<std::thread> threads;
    threads.reserve(nthreads - 1);

    std::atomic<size_t> blocksRemaining;
    blocksRemaining.store(nblocks);

    // current thread's work
    {
        int threadIdx = nthreads - 1;  // this thread is the last one
        // current thread will start the last block to avoid having to check partial block
        size_t processBlock = --blocksRemaining;

        // front/back
        SizeType blockFront = processBlock * blockSize;
        SizeType blockLength = processEnd - blockFront;

        // initialize other threads
        for (int i = 0; i < nthreads - 1; i++) {
            threads.push_back(
                    std::thread(
                            __thread_worker<RandomAccessIterator, SizeType>, i, nthreads,
                            std::ref(blocksRemaining), blockSize,
                            std::ref(barrier), first, len));
        }

        mas::concurrency::thread_group<std::vector<std::thread>> threadGroup(threads); // for closing off threads

        // process current block
        __down_block(first, len, blockFront, blockLength);
        barrier.set(threadIdx, -1);

        // continue processing other blocks
        __thread_worker(threadIdx, nthreads, blocksRemaining, blockSize,
                barrier, first, len);

        // done!!
        // std::cout << "DONE!" << std::endl;

    }

}

template<typename RandomAccessIterator, typename SizeType, typename Compare, typename MoveCallback>
void __down_block(RandomAccessIterator first, SizeType length,
        SizeType blockFront, SizeType blockLength, Compare& compare, MoveCallback& moved) {
    SizeType parent = blockFront + blockLength - 1;
    for (SizeType i = 0; i < blockLength; i++) {
        __down_heap(first, length, parent, __MAS_MOVE(*(first + parent)), parent,
                compare, moved);
        parent--;
    }
}


template<typename RandomAccessIterator, typename SizeType, typename Compare, typename MoveCallback>
void __thread_worker(int threadIdx, int nthreads,
        std::atomic<size_t>& blocksRemaining, SizeType blockSize,
        mas::concurrency::rolling_barrier& barrier,
        RandomAccessIterator first, SizeType len, Compare& compare, MoveCallback& moved) {

    bool complete = false;
    while (!complete) {
        size_t processBlock = --blocksRemaining;

        // safe check for negative value accounting for overflow
        if (processBlock + nthreads < nthreads) {
            complete = true;
            barrier.set(threadIdx, -1);
            break;
        }

        // we are processing block 'processBlock'
        SizeType blockFront = processBlock * blockSize;
        SizeType firstChild = 2 * blockFront + 1;

        // check that we can go
        barrier.set(threadIdx, len-blockFront-blockSize);
        SizeType waitFor = len - firstChild - 1;
        while (!barrier.poll(threadIdx, waitFor)) {
            std::this_thread::yield();
        }
        __down_block(first, len, blockFront, blockSize, compare, moved);
        barrier.set(threadIdx, -1);
    }
}

template<typename RandomAccessIterator, typename Compare, typename MoveCallback>
void parallel_make_heap(RandomAccessIterator first, RandomAccessIterator last,
        Compare compare, MoveCallback moved,
        typename std::iterator_traits<RandomAccessIterator>::difference_type blockSize,
        int maxThreads) {

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type SizeType;

    // starting with deepest parent, bubble down
    const SizeType len = last - first;
    SizeType processEnd = len / 2;

    // number of blocks
    size_t nblocks = processEnd / blockSize;
    if (processEnd % blockSize != 0) {
        nblocks++;
    }

    if (maxThreads <= 0) {
        maxThreads = std::max(std::thread::hardware_concurrency(), (unsigned int)2); // try 2 threads if hardware_concurrency returns 0
    }

    size_t nthreads = std::min(nblocks, (size_t)maxThreads);
    mas::concurrency::rolling_barrier barrier(nthreads);
    std::vector<std::thread> threads;
    threads.reserve(nthreads - 1);

    std::atomic<size_t> blocksRemaining;
    blocksRemaining.store(nblocks);

    // current thread's work
    {
        int threadIdx = nthreads - 1;  // this thread is the last one
        // current thread will start the last block to avoid having to check partial block
        size_t processBlock = --blocksRemaining;

        // front/back
        SizeType blockFront = processBlock * blockSize;
        SizeType blockLength = processEnd - blockFront;

        // initialize other threads
        for (int i = 0; i < nthreads - 1; i++) {
            threads.push_back(
                    std::thread(
                            __thread_worker<RandomAccessIterator, SizeType,
                                    Compare>, i, nthreads,
                            std::ref(blocksRemaining), blockSize,
                            std::ref(barrier), first, len,
                            std::ref(compare), std::ref(moved)));
        }

        mas::concurrency::thread_group<std::vector<std::thread>> threadGroup(threads); // for closing off threads

        // process current block
        __down_block(first, len, blockFront, blockLength, compare, moved);
        barrier.set(threadIdx, -1);

        // continue processing other blocks
        __thread_worker(threadIdx, nthreads, blocksRemaining, blockSize,
                barrier, first, len, compare, moved);

        // done!!
        // std::cout << "DONE!" << std::endl;

    }

}

#endif

}
}

#endif
