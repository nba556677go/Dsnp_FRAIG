/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;
//extern CirMgr *cirMgr;
// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
void CirGate::sweepgate(){
  for(size_t i = 0 ; i< _faninList.size() ; i++){
    size_t faninout = _faninList[i].gate()->getfanout().size();
    for(size_t j = 0 ; j <  _faninList[i].gate()->getfanout().size(); ++j){
      if(_faninList[i].gate()->getfanoutindex(j).gate()->getid() == this->getid()){
      _faninList[i].gate()->removefanout(j);continue;}
    } 
  }
  _faninList.clear();
  for(size_t i = 0 ; i< _fanoutList.size() ; i++){
    size_t fanoutin = _fanoutList[i].gate()->getfanin().size();
    for(size_t j = 0 ; j < fanoutin ; ++j){
      if(_fanoutList[i].gate()->getfanindex(j).gate()->getid() == this->getid()){
       _fanoutList[i].gate()->removefanin(j);
      }
    }
  }
  _fanoutList.clear();
}
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{ 
   rebuilddf();
  
 /*
  for(size_t i = 0 ; i < inlist.size() ; i++){
    if(inlist[i]->getdflag() == false){
      for(size_t j = 0 ; j < inlist[i]->getfanout().size();  j++){
        if(inlist[i] ->getfanoutindex(j).gate()->getdflag()== false ){
          inlist[i]->removefanout(j);  
        }
      }
    }
  }
  
  for(size_t i =0  ; i < dflist.size() ; i++){
    if(dflist[i]->getTypeStr() == "PI")continue;
    for(size_t j = 0 ; j < dflist[i]->getfanout().size();  j++){
      if(dflist[i] ->getfanoutindex(j).gate()->getdflag()== false && dflist[i] ->getfanoutindex(j).gate()->getTypeStr()!= "PO" ){
        dflist[i]->removefanout(j)  ;    
      }
    }
  }*/
  
  for(size_t i = 1 ;  i <= _total ; i++ ){
    if(gatelist[i] == 0)continue;
    if(gatelist[i]->getTypeStr()== "PI" ) continue;
    //if(gatelist[i]->getTypeStr()== "PO"){assert(0);}
    /*if(gatelist[i] -> getTypeStr() == "UNDEF"   ){
      cout<<"Sweeping: "<<gatelist[i]->getTypeStr()<<"("<<gatelist[i]->getid()<<") removed..."<<endl;
      delete gatelist[i]; gatelist[i] = 0;
    }*/
    else if(  gatelist[i]->getdflag()== false ){
      gatelist[i]->sweepgate();
      if(gatelist[i]->getTypeStr()== "AIG")_andnum-- ;  
      cout<<"Sweeping: "<<gatelist[i]->getTypeStr()<<"("<<gatelist[i]->getid()<<") removed..."<<endl;
      delete gatelist[i]; gatelist[i] = 0;
    }
  }
  
  rebuilddf();


}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{ 
  rebuilddf();
  for(size_t i = 0 ;  i< dflist.size() ; i++){
    bool be_del = false;
    dflist[i]-> opttraversal(be_del);
    if(be_del){
      //assert(gatelist[dflist[i]->getid()]->getTypeStr()== "AIG");
       if(gatelist[dflist[i]->getid()]->getTypeStr()== "AIG")_andnum--;
       gatelist[dflist[i]->getid()]->resetdfflag();
       delete gatelist[dflist[i]->getid()]; gatelist[dflist[i]->getid()] = 0;
    }
  }
  /*
  for(size_t i = 1 ;  i <= _total ; i++ ){
      if(gatelist[i] == 0)continue;
      if(gatelist[i]->getTypeStr()== "PI") continue;
      if(gatelist[i]->getTypeStr()== "UNDEF"){
        delete gatelist[i] ; gatelist[i] = 0 ;
      }
  }*/
  
  dfupdate = false; rebuilddf();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
 void 
 CirGate::opttraversal(bool & bedel){
   
  /*for(int i =0 ; i< _faninList.size() ; i++){
     if(!(_faninList[i]->gate()->isgloblref() )){
       _faninList[i]->gate()->setglobal();
       _faninList[i]->gate()->dfstraversal(dflist );
     }
   }*/
   
   if(_faninList.size()==2){
      for(size_t i = 0 ; i < _faninList.size() ; i++){
        if(_faninList[i].gate()->getTypeStr() =="CONST" ){
          this->rmfani_opti();
          bedel = true;
          if(_faninList[i].isInv()){
             cout<<"Simplifying: "<<_faninList[(2-i)-1].gate()->getid()<<" merging ";
             if(_faninList[(2-i)-1].isInv())cout<<"!" ; cout<< this->getid()<<"..."<<endl; 
             for(size_t j = 0 ; j <  _fanoutList.size() ; j++){
               // cout<<"fanoutgate "<<_fanoutList[j]->gate()->getid()<<endl;
                //cout<<"fanoutphase: "<<_fanoutList[j]->isInv()<<endl;
                //cout<< "faninphase " <<_faninList[(2-i)-1]->isInv()<<endl;
                
                mergegate(_faninList[(2-i)-1].gate() , _fanoutList[j].gate() , _fanoutList[j].isInv()^_faninList[(2-i)-1].isInv() );
             }
          }
          else {
            
             cout<<"Simplifying: "<<"0"<<" merging ";
              cout<< this->getid()<<"..."<<endl; 
             for(size_t j = 0 ; j <  _fanoutList.size() ; j++){
                int i = 0;
                mergegate(cirMgr->getGate(i) , _fanoutList[j].gate() , _fanoutList[j].isInv() );
             }
          }
          break;
        }
        else if(_faninList[0].gate() == _faninList[1].gate()){
          bedel = true;
          this->rmfani_opti();

          if(_faninList[0].isInv() == _faninList[1].isInv()){
            cout<<"Simplifying: "<<_faninList[0].gate()->getid()<<" merging ";
            if(_faninList[0].isInv())cout<<"!" ; cout<< this->getid()<<"..."<<endl;
            for(size_t j = 0 ; j <  _fanoutList.size() ; j++){
             // cout<<"fanoutgate "<<_fanoutList[j]->gate()->getid()<<endl;
              //cout<<"fanoutphase: "<<_fanoutList[j]->isInv()<<endl;
                //cout<< "faninphase " <<_faninList[(2-i)-1]->isInv()<<endl;
              mergegate(_faninList[0].gate() , _fanoutList[j].gate() ,_fanoutList[j].isInv()^_faninList[0].isInv()  );
            }
          }
          else{
            cout<<"Simplifying: "<<"0"<<" merging ";
            cout<< this->getid()<<"..."<<endl;
            for(size_t j = 0 ; j <  _fanoutList.size() ; j++){
              int i = 0;
              mergegate(cirMgr->getGate(i) , _fanoutList[j].gate() , _fanoutList[j].isInv() );
            }
          }
          break;
        }       
      }
    }
 }



void
CirGate::mergegate(CirGate* bemerged , CirGate* merge, bool outphase){
    
    bool mergephase = outphase  ;
    //mergefanin
  
  /*
    for(size_t i = 0 ; i < bemerged->getfanout().size(); i ++){
      if(bemerged->getfanoutindex(i)->gate()->getid() == this->getid()){
        delete bemerged->getfanoutindex(i); 
        bemerged->setfanout( i , new CirGateV(merge , mergephase));
      }
    }*/
    bemerged->setfanout( CirGateV(merge , mergephase));
    //mergefanout
    size_t fanicansize =  merge->getfanin().size();
    for(size_t j = 0 ;  j< fanicansize ; j++){
      if( merge->getfanindex(j).gate()->getid() == this->getid()){
         merge->removefanin(j) ; 
        //cout<<"mergephase2 "<<mergephase<<endl;
        merge->setfanin(j , CirGateV(bemerged , mergephase ));
      }
    }

  
}
