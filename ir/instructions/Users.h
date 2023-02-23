#ifndef JIT_AOT_COMPILERS_COURSE_USERS_H_
#define JIT_AOT_COMPILERS_COURSE_USERS_H_

#include "arena/ArenaAllocator.h"
#include <span>


namespace ir {
class InstructionBase;

class Users {
public:
    explicit Users(utils::memory::ArenaAllocator *const allocator)
        : users(allocator->ToSTL())
    {
        ASSERT(allocator);
    }
    Users(std::span<InstructionBase *> instrs, utils::memory::ArenaAllocator *const allocator)
        : users(instrs.begin(), instrs.end(), allocator->ToSTL())
    {
        ASSERT(allocator);
    }
    NO_COPY_SEMANTIC(Users);
    NO_MOVE_SEMANTIC(Users);
    virtual DEFAULT_DTOR(Users);

    size_t UsersCount() const {
        return users.size();
    }

    const utils::memory::ArenaVector<InstructionBase *> &GetUsers() const {
        return users;
    }
    std::span<InstructionBase *> GetUsers() {
        return std::span(users);
    }

    void AddUser(InstructionBase *instr) {
        ASSERT(instr);
        users.push_back(instr);
    }
    void AddUsers(std::span<InstructionBase *> instrs) {
        users.reserve(users.size() + instrs.size());
        for (auto &&it : instrs) {
            users.push_back(it);
        }
    }

    void RemoveUser(InstructionBase *instr) {
        auto iter = std::find(users.begin(), users.end(), instr);
        ASSERT(iter != users.end());
        *iter = users.back();
        users.pop_back();
    }

    void ReplaceUser(InstructionBase *oldInstr, InstructionBase *newInstr) {
        auto iter = std::find(users.begin(), users.end(), oldInstr);
        ASSERT(iter != users.end());
        *iter = newInstr;
    }

    void SetNewUsers(utils::memory::ArenaVector<InstructionBase *> &&newUsers) {
        users = std::move(newUsers);
    }

protected:
    utils::memory::ArenaVector<InstructionBase *> users;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_USERS_H_
