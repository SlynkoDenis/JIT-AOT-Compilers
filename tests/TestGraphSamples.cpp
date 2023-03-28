#include "TestGraphSamples.h"


namespace ir::tests {
TestGraphSamples::CFGInfoPair TestGraphSamples::BuildCase0() {
    /*
       A
       |
       B
      / \
     /   \
    C     D
     \   /
      \ /
       E
       |
       F
    */
    auto *graph = GetGraph();
    std::vector<BasicBlock *> bblocks(6);
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBasicBlock();
    }
    graph->SetFirstBasicBlock(bblocks[0]);
    graph->SetLastBasicBlock(bblocks[5]);

    graph->ConnectBasicBlocks(bblocks[0], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[2], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[3], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[5]);

    return {graph, bblocks};
}

TestGraphSamples::CFGInfoPair TestGraphSamples::BuildCase1() {
    /*
       A
       |
       B
      / \
     /   \
    C     F
    |    / \
    |   E   \
    |  /    |
    | /     |
    D<------G
    */
    auto *graph = GetGraph();
    std::vector<BasicBlock *> bblocks(7);
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBasicBlock();
    }
    graph->SetFirstBasicBlock(bblocks[0]);
    graph->ConnectBasicBlocks(bblocks[0], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[5]);
    graph->ConnectBasicBlocks(bblocks[2], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[6]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[3]);

    return {graph, bblocks};
}

TestGraphSamples::CFGInfoPair TestGraphSamples::BuildCase2() {
    /*
        A
        |
    --->B-->J
    |   |  /
    |   | /
    |   |/
    |   C<--
    |   |  |
    |   D---
    |   |
    |   E<--
    |   |  |
    |   F---
    |   |
    H<--G-->I-->K
    */
    auto *graph = GetGraph();
    std::vector<BasicBlock *> bblocks(11);
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBasicBlock();
    }
    graph->SetFirstBasicBlock(bblocks[0]);
    for (size_t i = 0; i < 7; ++i) {
        graph->ConnectBasicBlocks(bblocks[i], bblocks[i + 1]);
    }
    graph->ConnectBasicBlocks(bblocks[3], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[7], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[9]);
    graph->ConnectBasicBlocks(bblocks[9], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[8]);
    graph->ConnectBasicBlocks(bblocks[8], bblocks[10]);

    return {graph, bblocks};
}

TestGraphSamples::CFGInfoPair TestGraphSamples::BuildCase3() {
    /*
          A
          |
          |
    ----->B
    |    /|
    |   / |
    |  E  C<--
    |  |\ |  |
    |  | \|  |
    ---F  D  |
       |  |  |
       |  |  |
       H->G---
        \ |
         \|
          I
    */
    auto *graph = GetGraph();
    std::vector<BasicBlock *> bblocks(9);
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBasicBlock();
    }
    graph->SetFirstBasicBlock(bblocks[0]);
    for (size_t i = 0; i < 3; ++i) {
        graph->ConnectBasicBlocks(bblocks[i], bblocks[i + 1]);
    }
    graph->ConnectBasicBlocks(bblocks[1], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[5]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[7]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[7], bblocks[6]);
    graph->ConnectBasicBlocks(bblocks[7], bblocks[8]);
    graph->ConnectBasicBlocks(bblocks[3], bblocks[6]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[8]);

    return {graph, bblocks};
}

TestGraphSamples::CFGInfoPair TestGraphSamples::BuildCase4() {
    /*
         A
         |
         V
    ---->B
    |   / \
    |  /   \
    --C     D
            |
            V
            E
    */
    auto *graph = GetGraph();
    std::vector<BasicBlock *> bblocks(5);
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBasicBlock();
    }
    graph->SetFirstBasicBlock(bblocks.front());
    graph->SetLastBasicBlock(bblocks.back());

    graph->ConnectBasicBlocks(bblocks[0], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[2], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[3], bblocks[4]);

    return {graph, bblocks};
}

TestGraphSamples::LivenessInfoTuple TestGraphSamples::FillCase1() {
    auto [graph, bblocks] = BuildCase1();
    auto type = OperandType::I32;
    auto *instrBuilder = GetInstructionBuilder(graph);

    auto *arg0 = instrBuilder->CreateARG(type);
    auto *arg1 = instrBuilder->CreateARG(type);
    auto *constZero = instrBuilder->CreateCONST(type, 0);
    auto *constOne = instrBuilder->CreateCONST(type, 1);
    instrBuilder->PushBackInstruction(
        bblocks[0],
        arg0, arg1, constZero, constOne);

    auto *constCmp = instrBuilder->CreateCMP(type, CondCode::GE, constZero, constOne);
    auto *constJcmp = instrBuilder->CreateJCMP();
    instrBuilder->PushBackInstruction(bblocks[1], constCmp, constJcmp);

    auto *phiInput1 = instrBuilder->CreateADD(type, arg0, constOne);
    instrBuilder->PushBackInstruction(bblocks[2], phiInput1);

    auto *cmp = instrBuilder->CreateCMP(type, CondCode::NE, arg0, constOne);
    auto *jcmp = instrBuilder->CreateJCMP();
    instrBuilder->PushBackInstruction(bblocks[5], cmp, jcmp);

    auto *phiInput2 = instrBuilder->CreateADDI(type, arg0, 2);
    instrBuilder->PushBackInstruction(bblocks[4], phiInput2);

    auto *phiInput3 = instrBuilder->CreateSUBI(type, arg1, 3);
    instrBuilder->PushBackInstruction(bblocks[6], phiInput3);

    auto *phi = instrBuilder->CreatePHI(
        type,
        {phiInput1->ToInput(), phiInput2->ToInput(), phiInput3->ToInput()},
        {bblocks[2], bblocks[4], bblocks[6]});
    auto *ret = instrBuilder->CreateRET(type, phi);
    instrBuilder->PushBackInstruction(bblocks[3], phi, ret);

    std::pmr::vector<LiveIntervals> linearOrder{
        {{{2, 28}}, arg0},
        {{{30, 32}, {4, 16}}, arg1},
        {{{6, 12}}, constZero},
        {{{8, 22}}, constOne},
        {{{12, 14}}, constCmp},
        {{{14, 16}}, constJcmp},
        {{{18, 20}}, phiInput1},
        {{{22, 24}}, cmp},
        {{{24, 26}}, jcmp},
        {{{28, 30}}, phiInput2},
        {{{32, 34}}, phiInput3},
        {{{34, 36}}, phi},
        {{{36, 38}}, ret}};

    return {graph, bblocks, linearOrder};
}

TestGraphSamples::LivenessInfoTuple TestGraphSamples::FillCase4() {
    auto [graph, bblocks] = BuildCase4();
    auto type = OperandType::I32;
    auto *instrBuilder = GetInstructionBuilder(graph);

    auto *constOne = instrBuilder->CreateCONST(type, 1);
    auto *constTen = instrBuilder->CreateCONST(type, 10);
    auto *constTwenty = instrBuilder->CreateCONST(type, 20);
    instrBuilder->PushBackInstruction(
        bblocks[0],
        constOne, constTen, constTwenty);

    auto *phi1 = instrBuilder->CreatePHI(type, {constOne}, {bblocks[0]});
    auto *phi2 = instrBuilder->CreatePHI(type, {constTen}, {bblocks[0]});
    auto *cmpEq = instrBuilder->CreateCMP(type, CondCode::EQ, phi2, constOne);
    auto *jcmp = instrBuilder->CreateJCMP();
    instrBuilder->PushBackInstruction(
        bblocks[1],
        phi2, phi1, cmpEq, jcmp);

    auto *mul = instrBuilder->CreateMUL(type, phi1, phi2);
    auto *sub = instrBuilder->CreateSUB(type, phi2, constOne);
    instrBuilder->PushBackInstruction(
        bblocks[2],
        mul, sub);
    phi1->AddPhiInput(mul, bblocks[2]);
    phi2->AddPhiInput(sub, bblocks[2]);

    auto *add = instrBuilder->CreateADD(type, constTwenty, phi1);
    auto *retvoid = instrBuilder->CreateRETVOID();
    instrBuilder->PushBackInstruction(
        bblocks[3],
        add, retvoid);

    std::pmr::vector<LiveIntervals> linearOrder{
        {{{2, 18}}, constOne},
        {{{4, 8}}, constTen},
        {{{20, 22}, {6, 14}}, constTwenty},
        {{{20, 22}, {8, 16}}, phi1},
        {{{8, 18}}, phi2},
        {{{10, 12}}, cmpEq},
        {{{12, 14}}, jcmp},
        {{{16, 20}}, mul},
        {{{18, 20}}, sub},
        {{{22, 24}}, add},
        {{{24, 26}}, retvoid}};

    return {graph, bblocks, linearOrder};
}
}   // namespace ir::tests
