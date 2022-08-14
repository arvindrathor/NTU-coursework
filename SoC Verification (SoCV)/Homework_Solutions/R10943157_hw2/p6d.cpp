/****************************************************************************
  FileName     [ testBdd.cpp ]
  PackageName  [ ]
  Synopsis     [ Define main() ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "bddNode.h"
#include "bddMgr.h"

using namespace std;

/**************************************************************************/
/*                        Define Global BDD Manager                       */
/**************************************************************************/
BddMgr bm;

/**************************************************************************/
/*                    Define Static Function Prototypes                   */
/**************************************************************************/
static void initBdd(size_t nSupports, size_t hashSize, size_t cacheSize);


/**************************************************************************/
/*                             Define main()                              */
/**************************************************************************/
int
main()
{
   initBdd(6, 127, 61);

   /*-------- THIS IS JUST A TEST CODE ---------*/
   BddNode a(bm.getSupport(1));
   BddNode b(bm.getSupport(2));
   BddNode c(bm.getSupport(3));
   BddNode d(bm.getSupport(4));
   BddNode e(bm.getSupport(5));
   BddNode f(bm.getSupport(6));

   BddNode g1 = b & d;
   // cout << "g1" << g1 << endl;

   BddNode g2 = e & (~c);
   // cout << "g2" << g2 << endl;

   BddNode g3 = a & b;
   // cout << "g3" << g3 << endl;

   BddNode g4 = c & g1;
   // cout << "g4" << g4 << endl;

   BddNode g5 = g1 | g2;
   cout << "g5" << g5 << endl;

   BddNode g6 = g2 | d;
   // cout << "g6" << g6 << endl;

   BddNode g7 = g6 & g3;
   // cout << "g7" << g7 << endl;

   BddNode g8 = g4 | g7;
   // cout << "g8" << g8 << endl;

   BddNode g5n = ~g5;
   cout << "~g5" << ~g5 << endl;

   // BddNode g5right = g5.getRightCofactor(5);
   // cout << "g5 right\n" << g5right << endl;

   BddNode g95  = g8 & f & g5;
   BddNode g95n = g8 & f & g5n;

   BddNode redundancyCheck = ~g5 & (g95 ^ g95n);
   cout <<"Redundancy Check" << redundancyCheck << endl;

   BddNode diff95 = g95 ^ g95n;
   cout << "Diff(g9|g5)" << diff95 << endl;
   /*----------- END OF TEST CODE ------------*/
}


/**************************************************************************/
/*                          Define Static Functions                       */
/**************************************************************************/
static void
initBdd(size_t nin, size_t h, size_t c)
{
   BddNode::_debugBddAddr = true;
   BddNode::_debugRefCount = true;

//   bm.reset();
   bm.init(nin, h, c);
}

