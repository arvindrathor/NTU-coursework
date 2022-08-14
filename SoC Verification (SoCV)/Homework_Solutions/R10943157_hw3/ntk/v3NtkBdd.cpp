/****************************************************************************
  FileName     [ v3NtkBdd.cpp ]
  PackageName  [ v3/src/ntk ]
  Synopsis     [ V3 Network to BDDs. ]
  Author       [ Cheng-Yin Wu ]
  Copyright    [ Copyright(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef V3_NTK_C
#define V3_NTK_C

#include "v3NtkHandler.h" // MODIFICATION FOR SoCV BDD
#include "v3Ntk.h"
#include "v3Msg.h"
#include "v3StrUtil.h"
#include "bddNodeV.h" // MODIFICATION FOR SoCV BDD
#include "bddMgrV.h" // MODIFICATION FOR SoCV BDD

extern BddMgrV* bddMgrV; // MODIFICATION FOR SoCV BDD

const bool V3Ntk::setBddOrder(V3NtkHandler* const handler, const bool& file) const {
  unsigned supportSize = getInputSize() + getInoutSize() + 2*getLatchSize();
  if(supportSize >= bddMgrV->getNumSupports()) {
    Msg(MSG_ERR) << "BDD Support Size is Smaller Than Current Design Required !!" << endl;
    return false;
  }
  // build support
  unsigned supportId = 1;
  for(unsigned i = 0, n = getInputSize(); i < n; ++i) {
    const V3NetId& ntkId = (file)? getInput(i) : getInput(n-i-1);
    bddMgrV->addBddNodeV(ntkId.id, bddMgrV->getSupport(supportId)());
    bddMgrV->addBddNodeV(handler->getNetNameOrFormedWithId(ntkId),
        bddMgrV->getSupport(supportId)());
    ++supportId;
  }
  for(unsigned i = 0, n = getInoutSize(); i < n; ++i) {
    const V3NetId& ntkId = (file)? getInout(i) : getInout(n-i-1);
    bddMgrV->addBddNodeV(ntkId.id, bddMgrV->getSupport(supportId)());
    bddMgrV->addBddNodeV(handler->getNetNameOrFormedWithId(ntkId),
        bddMgrV->getSupport(supportId)());
    ++supportId;
  }
  for(unsigned i = 0, n = getLatchSize(); i < n; ++i) {
    const V3NetId& ntkId = (file)? getLatch(i) : getLatch(n-i-1);
    bddMgrV->addBddNodeV(ntkId.id, bddMgrV->getSupport(supportId)());
    bddMgrV->addBddNodeV(handler->getNetNameOrFormedWithId(ntkId),
        bddMgrV->getSupport(supportId)());
    ++supportId;
  }
  // Next State
  for(unsigned i = 0, n = getLatchSize(); i < n; ++i) {
    const V3NetId& ntkId = (file)? getLatch(i) : getLatch(n-i-1);
    bddMgrV->addBddNodeV(handler->getNetNameOrFormedWithId(ntkId)+"_ns",
        bddMgrV->getSupport(supportId)());
    ++supportId;
  }

  // Constants
  for (uint32_t i = 0; i < getConstSize(); ++i) {
    assert(getGateType(getConst(i)) == AIG_FALSE);
    bddMgrV->addBddNodeV(getConst(i).id, BddNodeV::_zero());
  }

  return true;
}

void V3Ntk::buildNtkBdd() 
{
  // TODO: build BDD for ntk here
  // Perform DFS traversal from DFF inputs, inout, and output gates.
  // Collect ordered nets to a V3NetVec
  // Construct BDDs in the DFS order

  for(int i = 0; i < getLatchSize(); i++) {
    const V3NetId& ntkId = getLatch(i);
    buildBdd(getInputNetId(ntkId, 0));
  }
  for(int i = 0; i < getInoutSize(); i++) {
    const V3NetId& ntkId = getInout(i);
    buildBdd(ntkId);
  }
  for(int i = 0; i < getOutputSize(); i++) {
    const V3NetId& ntkId = getOutput(i);
    buildBdd(ntkId);
  }
  _isBddBuilt = true;
}

void V3Ntk::buildBdd(const V3NetId& netId) 
{
  V3NetVec orderedNets;

  orderedNets.clear();
  orderedNets.reserve(getNetSize());

  newMiscData();
  dfsOrder(netId, orderedNets);
  assert (orderedNets.size() <= getNetSize());

  // TODO: build BDD for the specified net here
  
  for (int i = 0 ; i < orderedNets.size(); i++)
  {
    V3NetId& DepthId = orderedNets[i];
    BddNodeV Node = bddMgrV -> getBddNodeV(DepthId.id);
    if(getGateType(DepthId) == AIG_NODE)
    {  
      assert(getInputNetSize(DepthId)==2);

      BddNodeV L0Node = bddMgrV -> getBddNodeV(getInputNetId(DepthId,0).id);
      BddNodeV R1Node = bddMgrV -> getBddNodeV(getInputNetId(DepthId,1).id);

      assert(L0Node != size_t(0));
      assert(R1Node != size_t(0));
      
      if(getInputNetId(DepthId,0).cp) L0Node = ~L0Node;
      if(getInputNetId(DepthId,1).cp) R1Node = ~R1Node;
      
      Node = L0Node & R1Node;
    }
    bddMgrV -> addBddNodeV(DepthId.id,Node());
  }
}

// put fanins of a net (id) into a vector (nets) in topological order
void V3Ntk::dfsOrder(const V3NetId& id, V3NetVec& nets) {
  if (isLatestMiscData(id)) return;
  // Set Latest Misc Data
  setLatestMiscData(id);
  // Traverse Fanin Logics
  const V3GateType type = getGateType(id);
  if(type == AIG_NODE) {
    dfsOrder(getInputNetId(id, 0), nets);
    dfsOrder(getInputNetId(id, 1), nets);
  }
  nets.push_back(id);  // Record Order
}

#endif
