#ifndef REAPER_MISC_SMARTPTR_H
#define REAPER_MISC_SMARTPTR_H

#include <memory>

namespace reaper {
namespace misc {

// Compatibility names for code that is being migrated incrementally. Ownership
// and weak-reference semantics are provided entirely by the C++ standard
// library; the former hand-written reference counter has been retired.
template<class T>
using SmartPtr = std::shared_ptr<T>;

template<class T>
using WeakPtr = std::weak_ptr<T>;

} // namespace misc
} // namespace reaper

#endif
