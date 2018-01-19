/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

//#include "cirDef.h"
#include "cirGate.h"
extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr(){
     _total =_innum =  _latnum = _outnum= _andnum =0 ;
   }
   ~CirMgr() {
     reset();
   }


   void reset(){
     for(size_t i = 0 ; i< gatelist.size() ; i++){
       if(gatelist[i] != 0){
        delete gatelist[i]; gatelist[i] = 0;
       }
       //delete g; g = 0;
     }
     
     gatelist.clear();
     inlist.clear();
     outlist.clear();
     aiglist.clear();
     dflist.clear();
   }

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   
   
   CirGate* getGate(int gid) const ;
     
   
   
   
   //void fanintraversal(int & , int & ,const CirGate*  , bool&);
   void readaigfan(int& , int& , int& );
   CirGate* findoutput(int &) const;
   CirGate* findinput(int &) const;
   
   // Member functions about circuit construction

   bool readCircuit(const string&);

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void writeAag(ostream&) const;
   void printFECPairs() const;
   void writeGate(ostream&, CirGate*) const;

   // Member functions about circuit optimization
   void sweep();
   void optimize();
   void rebuilddf();
   

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void fecinit();
   void writeSimLog(size_t ); 
   //void fecreset();
   void setSimLog(ofstream *logFile) { _simLog = logFile; }
   void updatefec();
   //void sortfec();
   vector<vector<CirGate*> >& getfecgroup(){return fecgroup[0];}
   void simulating(vector<size_t>&  );
   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();
   void genpfModel(SatSolver&   );
   bool solveCNF(SatSolver& , CirGate* , CirGate* , vector<pair<CirGate*, CirGate*> >& , vector<size_t>& );
   void merge_resetfec(vector<pair<CirGate*, CirGate*> >&);
   void  fectriversal(SatSolver& , vector<pair<CirGate* ,  CirGate* > >&  ,
   vector<size_t>&  ,pair<CirGate* , CirGate*>&  , bool&);
   void mergepair( vector<pair<CirGate* , CirGate*> >&);
   void resetfec();

private:
  size_t _total;
  size_t _innum;
  size_t _latnum;
  size_t _outnum;
  size_t _andnum;
  ofstream *_simLog;
  vector<CirGate*>  gatelist;
  //vector<unsigned> idlist;
  vector<unsigned> inlist;
  vector<CirGate*> outlist;
  vector<unsigned> aiglist;//prevent dupilcate
  vector<CirGate*> dflist;
  vector<vector<CirGate*> > fecgroup[2];
  bool dfupdate;
};

#endif // CIR_MGR_H