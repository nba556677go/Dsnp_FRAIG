/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;
typedef pair<HashKey, CirGate*> HashNode;
// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned long int simnum = 0;
static bool domerge = false;
static size_t convert2dig(size_t a  ){
   //cout<<a <<endl;
   bool conv[64];
   for(size_t i = 0 ; i < 64 ; i++){
     if(a>>(i) & (size_t)1) conv[i] = true;
     else conv[i] = false ;
     //if(i!=0 && i%8 ==0) cout<<"_";
   }
   for(int i = 63 ; i >=0 ; i--){
     cout<<conv[i];
     if( i%8 ==0&& i!=0) cout<<"_";
   }
  
 }
/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void 
CirGate::strashmerge(CirGate* mergeto  ){
  //remove fanin's fanout and this fanin
  for(size_t i = 0 ; i< _faninList.size() ; i++){
    //size_t faninout = _faninList[i].gate()->getfanout().size();
    for(size_t j = 0 ; j < _faninList[i].gate()->getfanout().size() ; ++j){
      if(_faninList[i].gate()->getfanoutindex(j).gate()->getid() == this->getid()){
        _faninList[i].gate()->removefanout(j);--j;}
    } 
  }
  _faninList.clear();
  //connect all fanout's fanin to newgate && newgate to fanout
  for(size_t i = 0 ; i< _fanoutList.size() ; i++){
    size_t fanoutin = _fanoutList[i].gate()->getfanin().size();
    for(size_t j = 0 ; j < fanoutin ; ++j){
      if(_fanoutList[i].gate()->getfanindex(j).gate()->getid() == this->getid()){
        CirGateV v = CirGateV(mergeto , _fanoutList[i].gate()->getfanindex(j).isInv());
        _fanoutList[i].gate()->setfaninV(j , v);
        //cout<<"should be mergeto: "<<v.gate()->getid()<<endl;
        //mergeto fanout connect to fanoutgate
        //if(!(mergeto->findoutgate(_fanoutList[i].gate()))){
          mergeto->setfanout(_fanoutList[i]);
        //}
        
        
      }
    }
  }
  _fanoutList.clear();
}

void
CirMgr::strash()
{
  rebuilddf();
  
   HashMap<HashKey , CirGate*> Hashmap(gatelist.size());
   
  for(size_t i = 0 ; i < dflist.size() ; i++){
    if(dflist[i]==0)continue;
    if(dflist[i]->getTypeStr()!= "AIG")continue;
    HashKey key =  HashKey(dflist[i]->getfanindex(0).getpri() ,  dflist[i]->getfanindex(1).getpri());
    CirGate* mergegate  = 0 ;
    if(Hashmap. query(key , mergegate)){
      dflist[i]->strashmerge(mergegate );
      cout<<"Strashing: "<<mergegate->getid()<<" merging ";
      cout<< dflist[i]->getid()<<"..."<<endl;
      
      if(gatelist[dflist[i]->getid()]->getTypeStr()== "AIG")_andnum--;
      gatelist[dflist[i]->getid()]->resetdfflag();
      delete gatelist[dflist[i]->getid()]; gatelist[dflist[i]->getid()] = 0;
    }
    else {Hashmap.mustinsert(key , dflist[i]); }
  }
  dfupdate = false;
  rebuilddf();
}

void
CirMgr::fraig()
{  
  rebuilddf();
  SatSolver s;
  s.initialize();
  genpfModel(s);
  vector<pair<CirGate* ,  CirGate* > > bemergedpair;
  vector<size_t> simkey;
  simkey.resize( _innum , (size_t) 0 );
  while(fecgroup[0].size() > 0){
    pair<CirGate* ,CirGate* >judgedel;
    fectriversal(s, bemergedpair , simkey , judgedel , domerge);
    if(judgedel.first ==0  && judgedel.second == 0){ 
      //run all dflist without finding can delete
      //cout<<"simkey "<<simkey.size()<<endl;
      mergepair(bemergedpair);
      simulating(simkey);
      cout<<'\r'<<"Updating by UNSAT... Total #FEC Group = "<<fecgroup[0].size()<<endl;
      simkey.clear(); simkey.resize( _innum , (size_t) 0 );
      continue;
    } 
    else if(simnum!=0 && simnum%64 ==0){
      //64simulate one time
      mergepair(bemergedpair);
      simulating(simkey);
      cout<<'\r'<<"Updating by SAT... Total #FEC Group = "<<fecgroup[0].size()<<endl;
      simkey.clear(); simkey.resize( _innum , (size_t) 0 );
    }
    else if(domerge){
      
      simulating(simkey);
      //mergepair(bemergedpair); 
      domerge = false;
      cout<<'\r'<<"Updating by SAT... Total #FEC Group = "<<fecgroup[0].size()<<endl;
    } 
    /*genpfModel(s);
    findSAT(s , bemergedpair ,simkey)
    if(simnum != 0 && simnum % 64 == 0){
      merge_resetfec(bemergedpair); simulating(simkey) ;simkey.clear();
      bemergedpair.clear(); 
    }*/
  }
  strash();
  //resetfec();
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void
CirMgr::genpfModel(SatSolver& s  ){
  rebuilddf();
  //initialize var(inorder of CONST ->PI -> dfs(AIG)->PO)
  Var v = s.newVar();
    gatelist[0] ->setvar(0);
  for (size_t i = 0; i < inlist.size(); ++i) {
      //if(gatelist[inlist[i]] == 0) continue;
      Var v = s.newVar();
      gatelist[inlist[i]] ->setvar(v) ;
   }

  for (size_t i = 0; i < dflist.size(); ++i) {
    if(dflist[i]->getTypeStr() == "AIG"){
        Var v = s.newVar();
        dflist[i] ->setvar(v) ;
      }
  }   

   for(size_t i = 0 ; i< outlist.size() ; i++){
     //assert(outlist[i]->getfanin().size()==1);
     Var v = s.newVar();
     outlist[i]->setvar(outlist[i]->getfanindex(0).gate()->getvar());
   }
//set aigate////////////////////////////////////////////
   for (size_t i = 0; i < dflist.size(); ++i) {
     if(dflist[i]->getTypeStr() == "AIG"){
       //assert(dflist[i]->getfanin().size() ==2);
       s.addAigCNF(dflist[i]->getvar(), dflist[i]->getfanindex(0).gate()->getvar(), dflist[i]->getfanindex(0).isInv(),
       dflist[i]->getfanindex(1).gate()->getvar(),dflist[i]->getfanindex(1).isInv() );
     } 
   }
   

}

void
CirMgr::fectriversal(SatSolver&s , vector<pair<CirGate* ,  CirGate* > >& bemergedpair ,vector<size_t>& simkey ,pair<CirGate* , CirGate*>& store, bool& gomerge){
  //solveconstFEC(s);
  
  //solve normal fec in n square
  for(size_t i = 0 ; i< dflist.size() ; i++){
    if(dflist[i]->getsmiupos()==INT64_MAX)continue;
    if(dflist[i]->getTypeStr() == "AIG"){
      
      for(int j = 0 ; j < fecgroup[0][dflist[i]->getsmiupos()].size()  ; j++){
         if(j == fecgroup[0][dflist[i]->getsmiupos()].size() -1){
            //domerge
            //cout<<"set to domerge ..."<<endl;
            gomerge = true;
          }
          if(fecgroup[0][dflist[i]->getsmiupos()][j] ==  dflist[i]){ continue;}
          //assert(!(fecgroup[0][dflist[i]->getsmiupos()][j]->getfraigdel()));continue;
         

          //setting var
          //cout<<"SATcandidate : ("<<  dflist[i]->getid()<<" , "<< fecgroup[0][dflist[i]->getsmiupos()][j]->getid()<<")"<<endl;
          
          if(solveCNF(s , dflist[i] , fecgroup[0][dflist[i]->getsmiupos()][j] , bemergedpair , simkey)){
            //remove from fecgroup
             
             fecgroup[0][dflist[i]->getsmiupos()][j]->setfraigdel();
            // fecgroup[0][dflist[i]->getsmiupos()][k]->setfraigdel();
             
              store = make_pair(dflist[i] , fecgroup[0][dflist[i]->getsmiupos()][j]);
              bemergedpair.push_back(store);
              //cout<<"bedel "<<  fecgroup[0][dflist[i]->getsmiupos()][j]->getfraigdel() <<" , "<<fecgroup[0][dflist[i]->getsmiupos()][k]->getfraigdel()<<endl; 
              //cout<<"delpair: "<<store.first->getid() << " "<<store.second->getid()<<endl;
              fecgroup[0][dflist[i]->getsmiupos()].erase(fecgroup[0][dflist[i]->getsmiupos()].begin()+j);
              --j;
             
              //dflist[i]->resetsmiupos();
             return ;
          }
          //if nothing to del -> continue to find
          
        
      }
        
    }
  }
  //simulate final ones
 
   //pair<CirGate* , CirGate*> null( 0  , 0) ;
   CirGate* a = 0 ; CirGate* b = 0;
  store = make_pair( a , b);
 
   return ;
}

bool
CirMgr::solveCNF(SatSolver& s , CirGate* g1 , CirGate* g2 , vector<pair<CirGate*, CirGate*> >& bemerged , vector<size_t>& simkey){
   Var newv = s.newVar();
   bool judgeinv_pat= false;
   if(g1->_simpattern == ~(g2->_simpattern)){judgeinv_pat = true;}
   s.addXorCNF(newv , g1->getvar() ,false, g2->getvar() ,judgeinv_pat   );
   s.assumeRelease();  // Clear assumptions
   s.assumeProperty(newv, true);
   s.assumeProperty(gatelist[0]->getvar() , false);
   bool bedel;
   bedel = s.assumpSolve();
   //cout<<"bedel: "<<bedel<<endl;
   if(!bedel){//should be deleted
      return true;
   }
   else {
        //CEX ... generate simulate key everyinput has to go in
     //   cout<<"proving not equivalent"<<endl;
      for(size_t i = 0 ; i< simkey.size() ; i++){
     //   assert(gatelist[inlist[i]]->getvar()!= -1);
        
        simkey[i] =( simkey[i]<<1 | (size_t)(s.getValue(gatelist[inlist[i]]->getvar())));
        //cout<<"PI "<<i <<" ";
        //size_t t = convert2dig(simkey[i]);cout<<endl;
      }
      
      simnum++;
      return false;
   }
}
void  
CirMgr::mergepair( vector<pair<CirGate* ,  CirGate* > >& bemergedpair){

  //cout<<"inmerge..."<<endl;
  //cout<<"bemerged pair size: "<<bemergedpair.size()<<endl;
  
  for(size_t i = 0 ; i < bemergedpair.size() ; ++i ){
    //assert(bemergedpair[i].second->getTypeStr() == "AIG" );
    if(bemergedpair[i].second->getTypeStr() == "AIG")--_andnum;
    bemergedpair[i].first ->fraigmerge(bemergedpair[i].second);
    delete gatelist[bemergedpair[i].second->getid()];
    gatelist[bemergedpair[i].second->getid()] = 0;
    
  } 
  bemergedpair.clear();
  
 
  rebuilddf();
} 

void
CirGate::fraigmerge(CirGate* bemerged ){
  //this merging bemerged...
  //assert(_faninList.size()==2);
  bool phase ;
  if(this->_simpattern == ~(bemerged->_simpattern)){phase = true; } 
  else {phase = false;}
  cout<<"Fraig: "<<this->getid() <<" merging ";
  if(phase)cout<<"!"; cout<<bemerged->getid()<<"..."<<endl;
  for(size_t i = 0 ; i< bemerged->getfanin().size() ; i++){
    for(size_t j = 0 ; j < bemerged->getfanindex(i).gate()->getfanout().size() ; ++j){
      if(bemerged->getfanindex(i).gate()->getfanoutindex(j).gate()->getid() == bemerged->getid()){
        bemerged->getfanindex(i).gate()->removefanout(j);--j;}
    } 

  }
  bemerged->getfanin().clear();
  
  for(size_t i  = 0 ;  i< bemerged->getfanout().size() ; i++){
    for(size_t j  = 0 ;  j< bemerged->getfanoutindex(i).gate()->getfanin().size() ; j++){
        if(bemerged->getfanoutindex(i).gate()->getfanindex(j).gate()->getid() == bemerged->getid()  ){
          //see two gates var to decide phase ...
          CirGateV v1 = CirGateV(this , phase);
          bemerged->getfanoutindex(i).gate()->setfaninV(j , v1);
          CirGateV v2 = CirGateV(bemerged->getfanoutindex(i).gate(), phase);
          this->setfanout(v2);
        }
    }
  }
  bemerged->getfanout().clear();

 
 
}


 void 
 CirMgr::resetfec(){
  for(size_t i = 0 ; i < fecgroup[0].size() ; i++){
    for(size_t j = 0 ; j < fecgroup[0][i].size() ; j++){
      fecgroup[0][i][j]->resetfechead();
      fecgroup[0][i][j]->resetsmiupos();
      //fecgroup[0][i][j]->_simpattern = (size_t)0;
    }
  }
  fecgroup[0].clear();
  fecgroup[1].clear();
 }
