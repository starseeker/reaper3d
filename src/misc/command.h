#ifndef REAPER_MISC_COMMAND_H
#define REAPER_MISC_COMMAND_H

#include <functional>
#include <type_traits>
#include <utility>

namespace reaper {
namespace misc {

struct Null { };

template<class R, int argument_count = 0, class Arg = Null>
using Command = std::conditional_t<
	argument_count == 0,
	std::function<R()>,
	std::function<R(Arg)>>;

template<class R, class Callable>
Command<R> mk_cmd(Callable callable)
{
	return [callable = std::move(callable)]() mutable -> R {
		return callable();
	};
}

template<class S, class T, class R>
Command<R> mk_cmd(S* object, R (T::*method)())
{
	return [object, method]() -> R {
		return (object->*method)();
	};
}

template<class S, class T, class A, class R>
Command<R> mk_cmd(S* object, R (T::*method)(A), A argument)
{
	return [object, method, argument = std::move(argument)]() mutable -> R {
		return (object->*method)(argument);
	};
}

template<class S, class T, class A, class R>
Command<R> mk_cmd(S* object, R (T::*method)(const A&), A argument)
{
	return [object, method, argument = std::move(argument)]() mutable -> R {
		return (object->*method)(argument);
	};
}

template<class S, class T, class A, class R>
Command<R, 1, A> mk_cmd(S* object, R (T::*method)(A))
{
	return [object, method](A argument) -> R {
		return (object->*method)(argument);
	};
}

} // namespace misc
} // namespace reaper

#endif
