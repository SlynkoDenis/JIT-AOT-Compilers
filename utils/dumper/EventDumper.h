#ifndef JIT_AOT_COMPILERS_COURSE_EVENT_DUMPER_H_
#define JIT_AOT_COMPILERS_COURSE_EVENT_DUMPER_H_

#include "arena/ArenaAllocator.h"
#include "helpers.h"
#include "macros.h"
#include <sstream>
#include <type_traits>
#include <unordered_map>


namespace utils::dumper {
class EventDumper {
public:
    EventDumper(const std::string &name) : name(name) {}
    virtual DEFAULT_DTOR(EventDumper);

    template <bool WithNewLine = true, typename... ArgsT>
    void Dump(const ArgsT&... args) {
        preDump();
        utils::expand_t{(dump(std::move(toString(args))), void(), 0)...};
        if constexpr (WithNewLine) {
            dump("\n");
        }
    }

    const std::string &GetName() const {
        return name;
    }

    template <typename T = EventDumper, typename... ArgsT>
    static std::pair<bool, EventDumper *> AddDumper(
        memory::ArenaAllocator *const allocator,
        const std::string &name,
        ArgsT&&... args)
    requires std::is_base_of_v<EventDumper, std::remove_cv_t<std::remove_reference_t<T>>>
    {
        auto it = instances.find(name);
        if (it != instances.end()) {
            return {false, it->second};
        }
        return {true, createDumper<T>(allocator, name, std::forward(args)...)};
    }

    static EventDumper *GetDumper(const std::string &name) {
        auto it = instances.find(name);
        ASSERT(it != instances.end());
        return it->second;
    }

protected:
    virtual void preDump() {
        std::cout << '[' << GetName() << "]: ";
    }
    virtual void dump(std::string &&str) {
        std::cout << str;
    }

    template <typename T>
    static std::string toString(const T &value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

protected:
    std::string name;

private:
    NO_COPY_SEMANTIC(EventDumper);
    NO_MOVE_SEMANTIC(EventDumper);

    template <typename T = EventDumper, typename... ArgsT>
    static T *createDumper(memory::ArenaAllocator *const allocator, const std::string &name,
                           ArgsT&&... args)
    requires std::is_base_of_v<EventDumper, std::remove_cv_t<T>>
    {
        auto *dump = allocator->template NewRaw<T>();
        ASSERT(dump != nullptr);
        new(dump) T(name, std::forward(args)...);
        return reinterpret_cast<T*>(dump);
    }

private:
    static std::unordered_map<std::string, EventDumper*> instances;
};
}   // namespace utils::dumper

#endif // JIT_AOT_COMPILERS_COURSE_EVENT_DUMPER_H_
