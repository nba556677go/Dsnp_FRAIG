/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes

class CirGateV {
  
   #define NEG 0x1
public:
   CirGateV(CirGate* g, size_t phase): 
      _gateV(size_t(g) + phase) { }
   CirGate* gate() const {
      return (CirGate*)(_gateV & ~size_t(NEG)); }
   bool isInv() const { return (_gateV & NEG); }
   size_t getpri(){return _gateV;}
   void change(size_t gv){_gateV = gv;}
   int printinv() const{if(isInv()) return 1 ; return 0; }
  //isinv true == invert

  /*void printgatev(CirGateV* v) const{
    stringstream s;
    if(v->gate()->gettype() == UNDEF_GATE){
      s<<"*";
      if(v->isInv()) s<<"!";
    }
    else if
    s<< v->gate()->getid();
    cout<<s.str();
  }*/
private:
   size_t           _gateV;
};


class CirGate
{
  friend class cirMgr;
public:
   CirGate(size_t _ref , GateType _type  ,unsigned _line ,  unsigned _gateid , bool dflag = false , size_t simpattern = size_t(0), size_t _smiupos = INT64_MAX , bool _fechead =false   , bool _fraigdel = false):  
     _ref(_ref) , _type (_type) , _line(_line) , _gateid(_gateid) , dflag(dflag) ,  _simpattern(simpattern)   , _smiupos(_smiupos) , _fechead(_fechead) , _fraigvar((size_t)_gateid)  , _fraigdel(_fraigdel) {}
   
   virtual ~CirGate() {
     
      _iolist.clear();
      _faninList.clear();
      _fanoutList.clear();
   }
   static size_t    _globalRef_s;
   // Basic access methods
   virtual bool isAig() const = 0 ;
   void setdflag() {dflag = true;}
   void resetdfflag(){dflag = false;}
   bool getdflag(){return dflag;}

   virtual string getTypeStr() const { 
      
      switch(_type){
        case CONST_GATE: return "CONST"; break;
        case PI_GATE: return "PI"; break;
        case PO_GATE: return "PO"; break;
        case AIG_GATE: return "AIG"; break;
        case UNDEF_GATE: return "UNDEF"; break;
        case TOT_GATE: return "TOT" ; break;
      }
      return "";
    }
  void setfanin(CirGateV v){_faninList.push_back(v);}  
  void setfanout(CirGateV v){_fanoutList.push_back(v);}
  vector<CirGateV>& getfanin() {return _faninList;}
  vector<CirGateV>& getfanout() {return _fanoutList;}
  vector<unsigned> getiolist(){return _iolist;}
  unsigned& getioindex(int i){return _iolist[i];}
  CirGateV& getfanindex(int i) {return _faninList[i];}
  CirGateV& getfanoutindex(int i ){return _fanoutList[i];}
  void setfanin(size_t& i , CirGateV g){_faninList.insert(_faninList.begin()+i , g) ;}
  void setfaninV(size_t& i , CirGateV g){_faninList[i].change(g.getpri());}
  void setfanout(size_t& i , CirGateV g){_fanoutList[i] = g;}
  void setfanoutV(size_t& i ,CirGateV g ){_fanoutList[i].change(g.getpri());}
  void removefanout(const size_t & i ){
    if(_fanoutList.size() <= i ) assert(0);
    //delete _fanoutList[i]; _fanoutList[i] = 0 ;
    _fanoutList.erase(_fanoutList.begin()+i);
  }
   void removefanin(const size_t & i ){
    if(_faninList.size() <= i ) assert(0);
    //delete _fanoutList[i]; _fanoutList[i] = 0 ;
    _faninList.erase(_faninList.begin()+i);
  }
  //for sweep
  void sweepgate();
  //for optimize
   void mergegate(CirGate*  , CirGate* , bool );
   void rmfani_opti();
   void opttraversal(bool & );
  //for strash
  void strashmerge(CirGate*);
  bool findoutgate(CirGate*g);
   
  
  // for simulation
  void setpattern(size_t i ){_simpattern = i;}
  size_t simtriversal();
  size_t getsmiupos(){return _smiupos;}
  void setsmiupos(size_t i ){_smiupos = i; }
  void resetsmiupos(){_smiupos = INT64_MAX;}
  void setfechead(){_fechead = true;}
  void resetfechead(){_fechead = false;}
  const bool& getfechead(){return  _fechead;}

  //forfraig
  void setvar(Var i){ _fraigvar = i;}
  Var& getvar(){return _fraigvar;}
  void findSAT(SatSolver&);
  void setfraigdel(){_fraigdel = true;}
  bool getfraigdel() const { return _fraigdel;}
  void resetfraigdel() {_fraigdel = false;}
  void fraigmerge(CirGate*);
 
  //void resetvar(){ _fraigvar = ;}

  //for dfs
  void dfstraversal(vector<CirGate* > & );
  bool isgloblref()const {
    if(_ref == _globalRef_s ) return true;
    else return false;
  }
  void setglobal(){ _ref = _globalRef_s;}
  void fanintraversal(int  , int &   , bool , CirGate* )const;
  void fanoutraversal(int , int & ,  bool  , CirGate*   ) const;
 
   // Printing functions
   string getname(){ return _name;}
   void printname(){ if(_name != "")cout<<" (" <<_name<<")";}
   void printGate() const {
    
    stringstream s;
    if(_name!= "")
    s<< getTypeStr()<<"("<<_gateid<<")"<<"\""<<_name<<"\""<<", line "<<_line;
    else s<<getTypeStr()<<"("<<_gateid<<")"<<", line "<<_line;
    cout<<"= "<<setw(47)<<left<<s.str()<<"="<<endl;
   }

   void printgatev(CirGateV& v) const{
    stringstream s;
    if(v.gate()->gettype() == UNDEF_GATE){
      s<<"*";
      if(v.isInv()) s<<"!";
    }
    else if(v.isInv()) s<<"!";
    s<< v.gate()->getid();
    cout<<s.str();
   }
   void wrgatetraversal(vector<unsigned>&);
   void printgatefec()const;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) ;
  unsigned getLineNo() const {return _line;}
   unsigned getid() const{ return _gateid;}
   GateType gettype() const{return _type;}
   void setname(string& s) { _name = s;}
   
    vector<unsigned> _iolist;
      size_t _simpattern;
   
protected:
   //vector<CirGateV>  _faninList;
  
   
   size_t  _ref;
   string _name;
   unsigned _line;
   unsigned _gateid;
   GateType _type;
   bool dflag;
   vector<CirGateV>  _faninList;
   vector<CirGateV>  _fanoutList;
   size_t _smiupos;
   bool _fechead;
   Var _fraigvar;
   bool _fraigdel;
   
 
};


class PIGate : public CirGate
{
 public:
   PIGate(size_t _ref , GateType _type  ,unsigned _line ,  unsigned _gateid , bool dflag = false):
   CirGate(_ref  , PI_GATE , _line , _gateid) {} 
   //~PIGate(){}
  bool isAig() const { return false; }
  
  


};

class POGate : public CirGate
{
  public:
    POGate(size_t _ref , GateType _type ,unsigned _line ,  unsigned _gateid , bool dflag = false):
   CirGate(_ref  , PO_GATE , _line , _gateid) {} 
  // ~POGate(){}
   //void printGate() const {}
  bool isAig() const { return false; }
};

class AIGate : public CirGate
{
  public:
    AIGate(size_t _ref, GateType _type ,unsigned _line , unsigned _gateid , bool dflag = false):
   CirGate(_ref  , AIG_GATE , _line , _gateid) {} 
   //~AIGate(){}
   
   bool isAig() const { return true; }
   //void setflag(){sweepflag = true;}
   private:
   // bool sweepflag ; 
 
};

class ConstGate : public CirGate
{
  public:
   ConstGate(size_t _ref , GateType _type ,unsigned _line ,  unsigned _gateid , bool sweepflag = false):
   CirGate(_ref  , CONST_GATE , _line , 0) {} 
   //~ConstGate(){}
  // void printGate() const {}
  bool isAig() const { return false; }
 /* private:
  vector<CirGateV*>  _fanoutList;*/
};

class UndefGate : public CirGate
{
  public:
   UndefGate(size_t _ref , GateType _type ,unsigned _line ,  unsigned _gateid,  bool sweepflag = false):
   CirGate(_ref  , UNDEF_GATE , 0 , _gateid) { } 
   //~UndefGate(){}
  // void printGate() const {}
  bool isAig() const { return false; }
 //void setflag(){sweepflag = true;}
  private:
   

};



#endif // CIR_GATE_H
