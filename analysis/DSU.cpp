#include "DSU.h"


namespace ir {
BasicBlock *DSU::Find(BasicBlock *bblock) {
    ASSERT((bblock) && (bblock->GetId() < GetSize()));
    if (universum[bblock->GetId()] == nullptr) {    // return if it's the root
        return bblock;
    }
    compressUniversum(bblock);
    // return node with min semi-dominator in ancestors chain
    return labels->at(bblock->GetId());
}

void DSU::compressUniversum(BasicBlock *bblock) {
    auto anc = universum[bblock->GetId()];
    ASSERT(anc != nullptr);

    if (universum[anc->GetId()] == nullptr) {    // return if root
        return;
    }

    compressUniversum(anc);

    auto minForBBlock = labels->at(bblock->GetId());
    auto minForAnc = labels->at(anc->GetId());
    if (sdoms->at(minForAnc->GetId()) < sdoms->at(minForBBlock->GetId())) {
        labels->at(bblock->GetId()) = minForAnc;  // update min semi-dominator
    }
    universum[bblock->GetId()] = universum[anc->GetId()];    // link to the root
}
}   // namespace ir
