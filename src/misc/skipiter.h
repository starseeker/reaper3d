#ifndef REAPER_MISC_SKIPITER_H
#define REAPER_MISC_SKIPITER_H

#include <iterator>
#include <memory>

#include "misc/sequence.h"

namespace reaper {
namespace misc {


template <typename Iter, typename Pr>
class skip_iterator
{
	Iter i;
	Iter end;
	Pr pr;

	void skip() {
		while(i != end && pr(*i)) {
			++i;
		}
	}

public:
	using traits_type = std::iterator_traits<Iter>;
	using iterator_category = std::forward_iterator_tag;
	using value_type = typename traits_type::value_type;
	using difference_type = typename traits_type::difference_type;
	using pointer = typename traits_type::pointer;
	using reference = typename traits_type::reference;

	skip_iterator(const skip_iterator &si)
		: i(si.i), end(si.end), pr(si.pr) {}
	skip_iterator(const Iter &i_, const Iter &end_, const Pr &p)
		: i(i_), end(end_), pr(p) { skip(); }

	reference operator*() const { return *i; }
	pointer operator->() const { return std::addressof(*i); }

	skip_iterator& operator++() {
		if(i != end) {
			++i;
			skip();
		}
		return *this;
	}

	skip_iterator operator++(int) {
		skip_iterator clone = *this;
		++(*this);
		return clone;
	}

	bool operator==(const skip_iterator &t) const {
		return i == t.i;
	}

	bool operator!=(const skip_iterator &t) const {
		return i != t.i;
	}
};

template <typename Iter, typename Pr>
inline skip_iterator<Iter, Pr> skip(Iter i, Iter end, Pr pr)
{
	return skip_iterator<Iter, Pr>(i, end, pr);
}

template <typename Iter>
inline auto skip_eq(
	Iter i,
	Iter end,
	const typename std::iterator_traits<Iter>::value_type& t)
{
	using value_type = typename std::iterator_traits<Iter>::value_type;
	return skip(i, end, [t](const value_type& value) {
		return value == t;
	});
}

// skip-sequences

template <typename Iter, typename Pr>
inline Seq<skip_iterator<Iter, Pr> > skip_seq(Iter i, Iter end, Pr pr)
{
	return Seq<skip_iterator<Iter, Pr> >(skip(i, end, pr),
	                                     skip(end, end, pr));
}

template <typename Iter, typename Pr>
inline Seq<skip_iterator<Iter, Pr> > skip_seq(const Seq<Iter> &s, Pr pr)
{
	return Seq<skip_iterator<Iter, Pr> >(skip(s.first, s.second, pr),
	                                     skip(s.second, s.second, pr));
}

template <typename Iter>
inline auto skip_seq_eq(
	Iter i,
	Iter end,
	const typename std::iterator_traits<Iter>::value_type& t)
{
	return Seq<decltype(skip_eq(i, end, t))>
	        (skip_eq(i, end, t), skip_eq(end, end, t));
}

template <typename Iter>
inline auto skip_seq_eq(
	const Seq<Iter>& s,
	const typename std::iterator_traits<Iter>::value_type& t)
{
	return Seq<decltype(skip_eq(s.first, s.second, t))>
		(skip_eq(s.first, s.second, t), skip_eq(s.second, s.second, t));
}

}
}

#endif
