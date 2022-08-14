#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <math.h>
#include <algorithm>
#include "sat.h"
using namespace std;

class Gate
{
   public:
      Gate(unsigned i = 0, string name = ""): _gid(i), _name(name){}
      ~Gate() {}

      Var getVar() const { return _var; }
      void setVar(const Var& v) { _var = v; }
      string getName() const { return _name; }
      void setName(const string name) { _name = name; }
   private:
      unsigned   _gid;  // for debugging purpose...
      Var        _var;
      string     _name;
};
vector<Gate *> gates;

struct instruct {  
   string type;  
   int first; 
   int second;
};

instruct parser(string raw){
   size_t endInst = raw.find('(');
   string _type = raw.substr(0,endInst);
   size_t end1st = raw.find(',');
   int _1st = stoi(raw.substr(endInst+1,end1st-endInst-1));
   size_t end2nd = raw.find(')');
   int _2nd = stoi(raw.substr(end1st+2,end2nd-end1st-2));
   instruct temp = {_type,_1st,_2nd};
   return temp;
}

vector<Var> toBinary(SatSolver& s, int num, int bits=0){
   vector<Var> num2Var;
   int num10 = num;
   while( num10 > 0 ){
      if( num10%2 == 0){ num2Var.push_back(s.getZero()); }
      else{ num2Var.push_back(s.getOne()); }
      num10/=2;
   }
   int originSize = int(num2Var.size());
   if(bits !=0){
      if(bits>int(num2Var.size())){ //Here
         for(int i=0;i<bits-originSize;i++) num2Var.push_back(s.getZero());
      }
   }
   // reverse(num2Var.begin(), num2Var.end());
   return num2Var;
}

void initCircuit( int num=0, int length=1){
   // Init gates
   int id=0;
   // cout << "Initialize for " << num << " seat(s)." << endl;
   for( int i=0; i < num; i++){
      for( int j=0; j < length; j++){
         string name="";
         name += to_string(i); name += "["; name += to_string(j); name += ']';
         gates.push_back(new Gate(id,name));
         id++;
      }
   }
}

Var ors(SatSolver& s, vector<Var> orInputs, vector<bool> orInvs){
   int size = orInputs.size();
   assert(orInputs.size() == orInvs.size());
   Var temp = s.newVar();
   if(orInputs.size()==1) s.addOrCNF(temp,orInputs[0],orInvs[0],s.getZero(),false);
   else s.addOrCNF(temp,orInputs[0],orInvs[0],orInputs[1],orInvs[1]);
   Var result = temp;
   for(int i=2;i<size;i++){
      temp = s.newVar();
      s.addOrCNF(temp,result,false,orInputs[i],orInvs[i]);
      result = temp;
   }
   return result;
}
Var ands(SatSolver& s, vector<Var> andInputs, vector<bool> andInvs){
   int size = andInputs.size();
   assert(andInputs.size() == andInvs.size());
   Var temp = s.newVar();
   if(andInputs.size()==1) s.addAigCNF(temp,andInputs[0],andInvs[0],s.getOne(),false);
   else s.addAigCNF(temp,andInputs[0],andInvs[0],andInputs[1],andInvs[1]);
   Var result = temp;
   for(int i=2;i<size;i++){
      temp = s.newVar();
      s.addAigCNF(temp,result,false,andInputs[i],andInvs[i]);
      result = temp;
   }
   return result;
}

Var not1(SatSolver& s, Var in){
   Var result = s.newVar();
   s.addAigCNF(result,in,true,in,true);
   return result;
}

Var equalInc(SatSolver& s, vector<Var> ainc, vector<Var> b, int bits=1){
   vector<Var> vars;
   vector<bool> invs;
   assert(ainc.size()==(b.size()+1));
   for(unsigned i=0;i<b.size();i++){
      vars.push_back(s.newVar());
      invs.push_back(true);
      s.addXorCNF(vars[i], ainc[i], false, b[i], false);
   }
   vars.push_back(ainc[ainc.size()-1]); //Carry out should not happen.
   invs.push_back(true);
   Var result = ands(s, vars, invs);
   return result;
}
Var equal(SatSolver& s, int m1, int m2, int bits=1){
   vector<Var> vars;
   vector<bool> invs;
   for(int i=0;i<bits;i++){
      vars.push_back(s.newVar());
      invs.push_back(true);
      s.addXorCNF(vars[i], gates[bits*m1+i]->getVar(), false, gates[bits*m2+i]->getVar(), false);
      // cout<<gates[4*m1+i]->getName()<<" "<<gates[4*m2+i]->getName()<<endl;
   }
   Var result = ands(s, vars, invs);
   return result;
}

Var unequal(SatSolver& s, int m1, int m2, int bits=1){
   Var eq = equal(s, m1, m2, bits);
   Var result = not1(s, eq);
   return result;
}

Var lessThanConst(SatSolver& s, int m1, int number, int bits=1){
   vector<Var> num2Var;
   int num10 = number;
   while( num10 > 0 ){
      if( num10%2 == 0){ num2Var.push_back(s.getZero()); }
      else{ num2Var.push_back(s.getOne()); }
      num10/=2;
   }
   
   if(int(num2Var.size())>bits){ return s.getOne(); }//Here
   else if(int(num2Var.size())<bits){ return s.getZero(); }//Here
   else{
      vector<Var> bitcomp; vector<bool> invs;
      for(int i=0;i<bits;i++){
         Var temp = s.newVar();
         if(i==0){
            s.addAigCNF(temp,gates[bits*(m1+1)-1-i]->getVar(),true,num2Var[bits-1-i],false);
            bitcomp.push_back(temp); invs.push_back(false);
         }
         else{
            vector<Var> eqs; vector<bool> eqInvs;
            for(int j=0;j<i;j++){
               Var tempEQ = s.newVar();
               s.addXorCNF(tempEQ,gates[bits*(m1+1)-1-j]->getVar(),true,num2Var[bits-1-j],false);
               eqs.push_back(tempEQ); eqInvs.push_back(false);
            }
            Var tempLth = s.newVar();
            s.addAigCNF(tempLth,gates[bits*(m1+1)-1-i]->getVar(),true,num2Var[bits-1-i],false);
            eqs.push_back(tempLth); eqInvs.push_back(false);
            temp = ands(s,eqs,eqInvs);
            bitcomp.push_back(temp); invs.push_back(false);
         }
      }
      return ors(s, bitcomp, invs);
   }
}

Var initConstraint(SatSolver& s, int num=0, int bits=1){
   for (unsigned i = 0; i < gates.size(); i++){
      Var v = s.newVar();
      gates[i]->setVar(v);
   }
   vector<Var> constraints;
   vector<bool> _;
   for (int i=0; i < num; i++){
      for (int j=i+1; j< num; j++){
         // cout << i<< " and "<< j<<" are not equal!"<<endl; 
         constraints.push_back(unequal(s,i,j,bits));
         _.push_back(false);
      }
   }
   // cout << "Less than "<< num<<" !"<<endl; 
   for (int i=0; i < num; i++){
      constraints.push_back(lessThanConst(s,i,num,bits));
      _.push_back(false);
   }
   return ands(s,constraints,_);
}

Var lessThan(SatSolver& s, int m1, int m2, int bits=1){
   vector<Var> bitcomp; vector<bool> invs;
   for(int i=0;i<bits;i++){
      Var temp = s.newVar();
      if(i==0){
         s.addAigCNF(temp,gates[bits*(m1+1)-1-i]->getVar(),true,gates[bits*(m2+1)-1-i]->getVar(),false);
         bitcomp.push_back(temp); invs.push_back(false);
      }
      else{
         vector<Var> eqs; vector<bool> eqInvs;
         for(int j=0;j<i;j++){
            Var tempEQ = s.newVar();
            s.addXorCNF(tempEQ,gates[bits*(m1+1)-1-j]->getVar(),true,gates[bits*(m2+1)-1-j]->getVar(),false);
            eqs.push_back(tempEQ); eqInvs.push_back(false);
         }
         Var tempLth = s.newVar();
         s.addAigCNF(tempLth,gates[bits*(m1+1)-1-i]->getVar(),true,gates[bits*(m2+1)-1-i]->getVar(),false);
         eqs.push_back(tempLth); eqInvs.push_back(false);
         temp = ands(s,eqs,eqInvs);
         bitcomp.push_back(temp); invs.push_back(false);
      }
   }
   return ors(s, bitcomp, invs);
}

Var assign(SatSolver& s, int man, int seat, int bits=1){
   vector<Var> binary = toBinary(s,seat,bits);
   vector<Var> enor; vector<bool> inv;
   int length = int(binary.size());
   assert(length == bits);
   for(int i=0;i<bits;i++){
      Var temp = s.newVar();
      s.addXorCNF(temp,gates[man*bits+i]->getVar(),true,binary[i],false);
      enor.push_back(temp); inv.push_back(false);
   }
   return ands(s,enor,inv);
}

Var assignNot(SatSolver& s, int man, int seat, int bits=1){
   return not1(s,assign(s, man, seat, bits));
}

vector<Var> increment(SatSolver& s, vector<Var> A){
   vector<Var> S; 
   Var C = A[0];
   A.push_back(s.getZero());
   S.push_back(not1(s,A[0]));
   for(unsigned i=1;i<A.size();i++){
      Var tempSum = s.newVar();
      Var tempCarry = s.newVar();
      s.addXorCNF(tempSum,A[i],false,C,false);
      S.push_back(tempSum);
      s.addAigCNF(tempCarry,A[i],false,C,false);
      C = tempCarry;
   }
   assert(S.size()==A.size());
   return S;
}

Var adjacent(SatSolver& s, int m1, int m2, int bits=1){
   vector<Var> m1bits; vector<Var> m2bits; vector<bool> _;
   for(int i=0;i<bits;i++){
      _.push_back(false);
      m1bits.push_back(gates[m1*bits + i]->getVar());
      m2bits.push_back(gates[m2*bits + i]->getVar());
   }
   vector<Var> m1bitsInc = increment(s, m1bits);
   vector<Var> m2bitsInc = increment(s, m2bits);
   Var result = s.newVar();
   s.addOrCNF(result,equalInc(s,m1bitsInc,m2bits),false,equalInc(s,m2bitsInc,m1bits),false);
   return result;
}
Var adjacentNot(SatSolver& s, int m1, int m2, int bits=1){
   return not1(s,adjacent(s, m1, m2, bits));
}
int main(int argc, char **argv)
{
   if (argc != 2) {
      cerr << "Error: Missing input file!!" << endl;
      exit(-1);
   }
   
   char* inputFile = argv[1];
   fstream fin;
 	string buffer;
 	fin.open(inputFile,ios::in);
 	if (!fin) {
 		cerr << "Error: Input file not found!!" << endl;
 		exit(1);
	}

   getline(fin,buffer);
   int seatNumber = stoi(buffer);
   int seatBitLength = 1;
   int temp = seatNumber-1;
   
   while(temp>=2){
      temp/=2;
      seatBitLength+=1;
   }
   
   initCircuit(seatNumber,seatBitLength);
   SatSolver solver;
   solver.initialize();
   vector<Var> allConstraints; vector<bool> _;
   allConstraints.clear();
   allConstraints.push_back(initConstraint(solver,seatNumber,seatBitLength)); _.push_back(false);
   
   while(!fin.eof()){
      getline(fin,buffer);
      if(buffer.size()==0) continue;
      instruct constraint = parser(buffer);
      // solver.addAigCNF(v0,t,false,adjacentNot(solver,constraint.first-1,constraint.second-1,seatBitLength),false);
      if(constraint.type == "Assign"){
         // cout << "Assign     : Man " << constraint.first << " must be seated on seat " << constraint.second << endl;
         allConstraints.push_back(assign(solver,constraint.first,constraint.second,seatBitLength)); _.push_back(false);
      }
      else if(constraint.type == "AssignNot"){
         // cout << "AssignNot  : Man " << constraint.first << " can't be seated on seat " << constraint.second << endl;
         allConstraints.push_back(assignNot(solver,constraint.first,constraint.second,seatBitLength)); _.push_back(false); 
      }
      else if(constraint.type == "LessThan"){
         // cout << "LessThan   : Man "<< constraint.first << " must be seated on a seat with the number less than seat of man " << constraint.second << endl;
         allConstraints.push_back(lessThan(solver,constraint.first,constraint.second,seatBitLength)); _.push_back(false);
      }
      else if(constraint.type == "Adjacent"){
         // cout << "Adjacent   : Man " << (constraint.first) << " must be seated adjacent to man " << (constraint.second)<<endl;
         allConstraints.push_back(adjacent(solver,constraint.first,constraint.second,seatBitLength)); _.push_back(false);
      }
      else if(constraint.type == "AdjacentNot"){
         // cout << "AdjacentNot: Man " << constraint.first << " can't be seated adjacent to man " << constraint.second<<endl;
         allConstraints.push_back(adjacentNot(solver,constraint.first,constraint.second,seatBitLength)); _.push_back(false);
      }
      else{
         cerr << "Error!!" << endl; 
      }
   }
   
   bool result;
   
   solver.assumeRelease();  // Clear assumptions
   Var v0 = solver.newVar();
   v0 = ands(solver,allConstraints,_);
   solver.assertProperty(v0,true);
   
   result = solver.solve();
   // solver.printStats();
   //cout << result << endl;
   if(result){
      cout<<"Satisfiable assignment:\n";
      int seatAssignment[seatNumber]; 
      int cal=0;
      for(unsigned i=0;i<gates.size();i++){
         //cout << solver.getValue(gates[i]->getVar());
         cal+=(solver.getValue(gates[i]->getVar())*pow(2,((i)%seatBitLength)));
         // cout << " "<< pow(2,((i)%seatBitLength)) <<"  "<<cal << endl;
         if((i+1)%seatBitLength==0) {
            // cout << " "<<cal<<" for "<<((i+1)/4-1);
            seatAssignment[cal]=((i+1)/seatBitLength-1);
            //cout << endl;
            cal=0;
         }
      }
      for(int i=0;i<seatNumber;i++){
         if(i==seatNumber-1) cout<<i<<"("<<seatAssignment[i]<<")\n";
         else cout<<i<<"("<<seatAssignment[i]<<"), ";
      }
   }
   
   else cout << "No satisfiable assignment can be found.\n";
}
