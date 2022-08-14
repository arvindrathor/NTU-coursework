/****************************************************************************
  FileName     [ proveBdd.cpp ]
  PackageName  [ prove ]
  Synopsis     [ For BDD-based verification ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2010-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include "v3NtkUtil.h"
#include "v3Msg.h"
#include "bddMgrV.h"

void
BddMgrV::buildPInitialState()
{
   // TODO : remember to set _initState
   // Set Initial State to All Zero

   V3Ntk* const NtkHand = v3Handler.getCurHandler() -> getNtk();

   // First reset _initState 
   _initState = BddNodeV::_one;

   // Build BDDs
   for(int i=0; i < NtkHand -> getLatchSize(); i++)
   {
     const V3NetId& LntkId = NtkHand -> getLatch(i);
     BddNodeV BddL = bddMgrV -> getBddNodeV(LntkId.id);
     _initState &= ~BddL;
   }
}

void
BddMgrV::buildPTransRelation()
{
   // TODO : remember to set _tr, _tri
   _tri = BddNodeV::_one;

   V3Ntk* const NtkHand = v3Handler.getCurHandler() -> getNtk();

   for(int i=0; i < NtkHand -> getLatchSize(); i++){
      const V3NetId LntkId = NtkHand -> getLatch(i);
      const V3NetId XIDelta_Id = NtkHand -> getInputNetId(LntkId,0);
      
		BddNodeV XIDelta = bddMgrV -> getBddNodeV(XIDelta_Id.id);
      
      if(XIDelta_Id.cp) 
      XIDelta = ~XIDelta;  

      BddNodeV Y = bddMgrV -> getBddNodeV(v3Handler.getCurHandler() -> getNetNameOrFormedWithId(LntkId) + "_ns");
      
      BddNodeV Ex_nor = ~((XIDelta) ^ Y);
      _tri &= Ex_nor;
   }

   _tr = _tri;

   for(unsigned inp = 1; inp < NtkHand -> getLatch(0).id ; inp++ ){
      _tr = _tr.exist(inp); 
  }
}

void
BddMgrV::buildPImage(int level)
{
   // TODO : remember to add _reachStates and set _isFixed
   // Note:: _reachStates record the set of reachable states
  
   V3Ntk* const NtkHand= v3Handler.getCurHandler() -> getNtk();
   unsigned CState =NtkHand-> getLatch(0).id;
   unsigned FFSize =NtkHand-> getLatchSize();
   
   // Loop in all levels
   for( int l = 0; l < level; l++ )
   {
      if(!_isFixed){
         BddNodeV LRState = getPReachState();
         BddNodeV QRState = LRState & _tr;
         
         // Exists Quantify
         unsigned range = FFSize + CState;
         for( unsigned input = CState; input < range; input++ ){
            QRState = QRState.exist(input);
         }
         // Replace Y with X
         bool move = false;
         if( QRState.getLevel() > 0 ){
            QRState = QRState.nodeMove( range, CState, move );
         }

         // Check Fixed Point Condition 
         if (LRState == QRState){
            Msg( MSG_IFO ) << "Fixed point is reached (time : " << _reachStates.size() << ")" << "\n";
            _isFixed = true;
            return;
         }
         
         _reachStates.push_back( QRState );
      }
      else{
         Msg( MSG_IFO ) << "Fixed point is reached (time : " << _reachStates.size() << ")" << "\n";
      }
   }
}

string convertStr(string s,int inpsize=1){
   string delimiter = " ";
   string conv[inpsize+1] = {"X"};
   for( int i = 0; i <= inpsize; ++i ) conv[i] = "1";
   size_t pos = 0;
   string token;
   if(s.size()>0){
      while ((pos = s.find(delimiter)) != string::npos) {
         token = s.substr(0, pos);
         int num;
         string numstr = ( s.substr(s.find("(")+1,s.find(")")-s.find("("))) ;
         sscanf(numstr.c_str(), "%d", &num);
         conv[num] = ((token[0] == '!') ? "0" : "1");
         s.erase(0, pos + delimiter.length());
      }
   }
   string ret = "";
   for(int i=1;i<=inpsize;i++){
      ret += conv[i];
   }
   // cout << conv[0]<<endl;
   return ret;
}

void 
BddMgrV::runPCheckProperty( const string &name, BddNodeV monitor )
{
   // TODO : prove the correctness of AG(~monitor)
   V3Ntk* const NtkHand= v3Handler.getCurHandler() -> getNtk();
   unsigned FFSize = NtkHand-> getLatchSize();
   unsigned inputSize = NtkHand-> getInputSize();

   unsigned X_Start = NtkHand-> getLatch(0).id;   
   unsigned Y_Start = FFSize + X_Start;
   unsigned Y_End = 2*FFSize -1 + X_Start; 
   
   BddNodeV Counter_Ex = monitor & getPReachState();

   if( Counter_Ex != BddNodeV::_zero ){ //Deal with Incorrect
      
      Msg( MSG_IFO ) << "Monitor \"" << name << "\" is violated.\n";
      Msg( MSG_IFO ) << "Counter Example:\n";
      
      // Find first timeframe yields counter examples.
      int BackTrackLvl = _reachStates.size()-1;
      
      for(unsigned i = 0 ; i < _reachStates.size(); i++)
      {
         if( ( _reachStates[BackTrackLvl - i] & monitor ) == BddNodeV::_zero )
         {
            BackTrackLvl += 1 - i;
            break;
         }
         else if( i == _reachStates.size()-1 )
         {
            BackTrackLvl = 0;
         }
      }
      
      BddNodeV Reach_XY = ( monitor & _reachStates[ BackTrackLvl ]).getCube();
      BddNodeV Get_IP = Reach_XY;
      for (unsigned i=X_Start;i <= Y_Start; i++ ) Get_IP = Get_IP.exist(i);

      for (unsigned i=1;i < X_Start; i++ ) Reach_XY = Reach_XY.exist(i);
      
      string counters[BackTrackLvl + 2] = {};

      counters[BackTrackLvl+1] = Get_IP.toString();
      BddNodeV Ct;
      for(int j = 0; j < BackTrackLvl+1; j++){
         bool move = false;

         // Move from X to Y
         Ct = Reach_XY.nodeMove(X_Start,Y_Start,move);
         if(move){
            
            if( BackTrackLvl-j-1 < 0 ) Ct = (_tri & _initState & Ct).getCube();
            else Ct = (_tri & Ct & _reachStates[ BackTrackLvl-j-1 ]).getCube();
            
            Get_IP = Ct;
            
            for(unsigned XY = X_Start; XY <= Y_End; XY++) Get_IP = Get_IP.exist(XY);

            counters[BackTrackLvl-j] = Get_IP.toString();

            Reach_XY = Ct;
            for(unsigned i=1; i < X_Start; i++) 
            Reach_XY=Reach_XY.exist(i);

            for(unsigned i=Y_Start; i <= Y_End; i++) 
            Reach_XY=Reach_XY.exist(i);
         }
      }
      
      for(int i=0; i< BackTrackLvl + 2; i++ ) 
      cout << i << ": " << convertStr(counters[i],inputSize) << "\n";
   }   
   
   else //Correct case
   {    if( _isFixed ) Msg( MSG_IFO ) << "Monitor \"" << name << "\" is safe.\n";

      else Msg( MSG_IFO ) << "Monitor \"" << name << "\" is safe up to time " << _reachStates.size() << ".\n";
   }
}
   



