/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <math.h>
#include <stdio.h>
#include <bitset>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions
#define const0 (size_t)0
//#define fecgroup[0] prifec
//#define fecgroup[1] tmpfec

/*******************************/
/*   Global variable and enum  */
/*******************************/
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
/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
size_t tosizet(char & i){
  if(i == '1'){return (size_t)1;}
  if(i == '0'){return (size_t)0;} 
  
}
void
CirMgr::simulating(vector<size_t>& readbf ){
    //start simulate//////////////////////
  
  
  
  //cout<<inlist.size() <<" "<<readbf.size()<<endl;
  //assert(inlist.size() == readbf.size()) ;
  if(fecgroup[0].size()==0){  fecinit();}
   CirGate::_globalRef_s++;
    //setconst
    gatelist[0]->setglobal();
    gatelist[0]->setpattern((size_t)0);
    for(size_t i = 0 ; i < inlist.size() ; i++){
      gatelist[inlist[i]]->setglobal(); 
      gatelist[inlist[i]]->setpattern(readbf[i]);
    }
    for(size_t i = 0 ;  i< outlist.size() ; i++){
      size_t simout = size_t(0);
      gatelist[outlist[i]->getid()]->setpattern(size_t(0));
      simout = gatelist[outlist[i]->getid()]->simtriversal();
    }
    updatefec();
    //cout<<"Total #FEC Group = "<< fecgroup[0].size()<<flush;
}
void
CirMgr::randomSim()
{ 
  rebuilddf();
  //bool fecinit =false;
  //vector<size_t> getrandom;
  if(fecgroup[0].size()==0){  fecinit();}
  
  //HashMap<SimKey , CirGate*> Simmap(gatelist.size());
  int runlimit = 50000;
  size_t fecstable = 0;
  size_t _totalnum = 0;
  //generating seed
  size_t orisize = 0;
  while( fecstable !=700 ){
    //if(fecstable > 150) break;
    CirGate::_globalRef_s++;
    //setconst
    gatelist[0]->setglobal();
    gatelist[0]->setpattern((size_t)0);
    for(size_t i = 0 ; i < inlist.size() ; i++){
      gatelist[inlist[i]]->setglobal();
      /*size_t shiftkey = (size_t)(rnGen(INT_MAX));
      shiftkey  =  shiftkey | (size_t)(rnGen(INT_MAX)) ;*/
      size_t shiftkey = (size_t)(((size_t)((size_t)(rnGen(INT_MAX))<<32))|((size_t)(rnGen((size_t)INT_MAX))));
      //cout<<'\r'<<"randomkey: "; size_t j =convert2dig(shiftkey); cout<<endl;
      gatelist[inlist[i]]->setpattern(shiftkey);
    }
    for(size_t i = 0 ;  i< outlist.size() ; i++){
      size_t simout = size_t(0);
      gatelist[outlist[i]->getid()]->setpattern(size_t(0));
      simout = gatelist[outlist[i]->getid()]->simtriversal();
    }
    if(_simLog != 0 ){
    writeSimLog(64);}
    orisize = fecgroup[0].size();
    updatefec();
    if(abs(orisize-fecgroup[0].size())< 5  ){fecstable ++;}
    _totalnum+= 64;
  
    //cout<<"runloop"<<endl;
    /*
    cout<<"while time: "<<runlimit<<endl;
    for(size_t l =0 ; l < fecgroup.size(); l++){
      cout<<"["<<l<<"]";
    for(size_t k =0 ;  k< fecgroup[l].size() ; k++){
      cout<<" "<<fecgroup[l][k]->getid();
    }
      cout<<endl;
    }*/
    //cout<<'\r'<<"Total #FEC Group"<< fecgroup[0].size()<<flush;
  }

  cout<<'\r'<<_totalnum<<" patterns simulated."<<endl;
  
  
  rebuilddf();
  
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  size_t totpattern = 0;
  rebuilddf();
  if(!patternFile.good()) return ;
    string eachline;
    vector<size_t>readbf;
  size_t line = 0 ;
  while(patternFile >>eachline){
      if(_innum != eachline.size()){
          cerr<< "Error: Pattern("<<  eachline<< ") length("<< eachline.size() <<") does not match the number of inputs("<<_innum<<") in a circuit!!"<<endl;
          //fecreset();
          cout<< '\r'<<"0 patterns simulated."<<endl;
          return;
        }
        for(size_t i = 0; i< eachline.size() ; i++){
          if(eachline[i]!='0' && eachline[i] != '1'){
            cerr<< "Error: Pattern("<<  eachline<<")contains a non-0/1 contains a non-0/1 character('"<<eachline[i]<<"')."<<endl;
            //fecreset();
            cout<<'\r'<<"0 patterns simulated."<<endl;
            return;
          }
        }

      //cout<<eachline<<endl;
      if(line%64==0  ){  
          //if line != 0 simulate
        if(line!= 0){
          simulating(readbf);
          if(_simLog != 0 ){
          writeSimLog(64);}
          
          readbf.clear();
        }
        //initialize
        readbf.resize(eachline.size() ,size_t(0));
      
        for(size_t i = 0; i< eachline.size() ; i++){
          readbf[i] |= (tosizet(eachline[i]));
        }
        eachline = "";
        line++; continue;
      }
      //assert(line > 0);
      for(size_t i = 0; i< eachline.size() ; i++){
          readbf[i] |= (tosizet(eachline[i])<< line);
      }
      ++line; eachline = "";
  }

    /*for(int i = 0 ; i < readbf.size() ; i ++){
        cout<<readbf[i]<<endl;
    }*/
    if(readbf.size()!=0){
      simulating(readbf);
      if(line%64!= 0 ){
         if(_simLog != 0 ){
        writeSimLog(line%64);}
      }
      else if(_simLog != 0 ){writeSimLog(64);}
    }
    //size_t remainnum = readbf.size();
    cout<<'\r' <<line<<" patterns simulated."<<endl;
     rebuilddf();
  //padding
  /*if(line<=63){
    for(size_t i = 0 ;  i< readbf.size() ; i++){
      readbf[i] |= (size_t) 0 ;
    }
  }*/
  //parse error
  /*if(floor(log2(readbf[0]))+1  != _innum ){
    size_t i = floor(log2(readbf[0])) ;
    i++;
    cerr<<"Error: Pattern( ) length("<< i <<") does not match the number of inputs("
    <<_innum<<"( in a circuit!!"<<endl;
  }*/
  

}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
static bool 
fecsort(CirGate* a  , CirGate* b ){
  return a->getid() < b->getid();
}
void 
CirMgr::fecinit(){
  fecgroup[0].clear();
  fecgroup[1].clear();
  vector<CirGate*> a;
  fecgroup[0].push_back(a);
  rebuilddf();
  //force const into fec
  fecgroup[0][0].push_back(gatelist[0]);
  std::sort(dflist.begin() , dflist.end() ,fecsort );
  for(size_t i  = 0 ;  i < dflist.size() ; i++){
    if( dflist[i]->getTypeStr() =="AIG" ){
      //if(dflist[i]->getTypeStr() == "CONST")
      fecgroup[0][0].push_back(dflist[i]);
    }
  }
  rebuilddf();
  /*
  for(size_t i  = 0 ;  i < dflist.size() ; i++){
    if(dflist[i]->getTypeStr() == "AIG")
    fecgroup[0].push_back(dflist[i]);
  }
  for(size_t i  = 0 ;  i < dflist.size() ; i++){
    if(dflist[i]->getTypeStr() == "PO")
    fecgroup[0].push_back(dflist[i]);
  }*/

}

size_t
CirGate::simtriversal(){
    if(this->isgloblref()) return _simpattern;
    
    if(this->getTypeStr() == "AIG"){
      this->setglobal();
      //for undef
      if(_faninList.size()== 0 ){_simpattern = 0 ; return _simpattern;}
      size_t tmp1; size_t tmp2 ;
       tmp1 = _faninList[0].gate()->simtriversal();
       tmp2  = _faninList[1].gate()->simtriversal();
      
      
      if(_faninList[0].isInv()) tmp1 = ~tmp1;
      if(_faninList[1].isInv()) tmp2 = ~tmp2;
      _simpattern = tmp1&tmp2;
      return _simpattern;
    }


    if(this->getTypeStr() == "PO"){
      this->setglobal();
      //assert(_faninList.size()==1);
         _simpattern = _faninList[0].gate()->simtriversal();
       if(_faninList[0].isInv()) _simpattern = ~_simpattern;
        return _simpattern;
    }
  }
    
void 
CirMgr::updatefec(){
  //tmpfecclear
  fecgroup[1].clear();
  
  //cout<<"should be fecsize "<<fecgroup.size()<<endl;
  
  //remove all size=1 element in tmpfec
  //unsigned fordel = 0;
  
  size_t originfecsize = fecgroup[0].size();
  for(size_t i = 0 ;  i< originfecsize ; i++){
    if(fecgroup[0][i].size()==1){continue;}
    //every row contruct onemap
    size_t eachrowsize = fecgroup[0][i].size();
    HashMap<SimKey , CirGate*> Simmap(eachrowsize *5);
    for(size_t j = 0 ; j < eachrowsize ; j++){//query every element
      SimKey key1(fecgroup[0][i][j]->_simpattern );
      //SimKey key2(~(fecgroup[i][j]->_simpattern)) ;
      CirGate* bequery1 = 0;
     
      if(!Simmap.query(key1 ,bequery1 )){
          
          //cout<<" inserting key to: "<< Simmap.checkindex(key1)<<endl;
            Simmap.mustinsert(key1 ,fecgroup[0][i][j] ); 
            vector<CirGate*> a; fecgroup[1].push_back(a);
            fecgroup[1].back().push_back(fecgroup[0][i][j]); 
            fecgroup[0][i][j]->setsmiupos(fecgroup[1].size()-1);
            //cout<<"inserting: "<<fecgroup[0][i][j]->getid()<<" to fecgroup "<<fecgroup[0][i][j]->getsmiupos() <<endl; 
            //cout<<" afterinsertsize(should be 1): "<< fecgroup[1][fecgroup[0][i][j]->getsmiupos()].size()<<endl;
      }
      else {
        //cout<<"in else"<<endl;
        //cout<<"checking key: " <<Simmap.checkindex(key1)<<endl;
        fecgroup[0][i][j]->setsmiupos(bequery1->getsmiupos() );
        fecgroup[1][bequery1->getsmiupos()].push_back(fecgroup[0][i][j]);
        //cout<<"inserting: "<<fecgroup[0][i][j]->getid()<<" to fecgroup "<<bequery1->getsmiupos()<<endl;
        //cout<<" afterinsertsize: "<< fecgroup[1][bequery1->getsmiupos()].size()<<endl;
      }
    } 
  }
  //remove size==1
  //remove all size=1 element in tmpfec 
  unsigned fordel = 0;
  while(fordel < fecgroup[1].size()){
     if(fecgroup[1][fordel].size()==1){
       fecgroup[1][fordel][0]->resetsmiupos();
       fecgroup[1][fordel][0]->resetfechead();
       //fecgroup[1][fordel][0]->resetinfec();
        swap(fecgroup[1][fordel], fecgroup[1].back());
        fecgroup[1].pop_back();
     }
     else {
        //assert(fecgroup[1][fordel].size() > 1);
        ++ fordel;
     }
  }
 
  for(size_t i =0 ; i < fecgroup[1].size(); i++){
    fecgroup[1][i][0]->setfechead();
    for(size_t j =0 ;  j< fecgroup[1][i].size() ; j++){
        fecgroup[1][i][j]->setsmiupos(i);
        //fecgroup[1][i][j]->setinfec();
     }
  }   
   swap(fecgroup[0] , fecgroup[1]);
   cout<<'\r'<<"Total #FEC Group = "<<fecgroup[0].size()<<flush;
  //cout<<char(13)<<setw(30)<<' '<<char(13);
  //////////////////////debug/////////////////////////////
  /*
   for(size_t i =0 ; i < fecgroup[0].size(); i++){
    cout<<"["<<i<<"]";
    for(size_t j =0 ;  j< fecgroup[0][i].size() ; j++){
      cout<<" ";
      if(fecgroup[0][i][j]->_simpattern == ~(fecgroup[0][i][0]->_simpattern) ){
        cout<<"!";
      }
      cout<<fecgroup[0][i][j]->getid()<<" flag infec: "<<fecgroup[0][i][j]->getinfec() ; 
      //cout<<" inrow: "<<fecgroup[0][i][j]->getsmiupos();
    }
    cout<<endl;
  }*/
}



void 
CirMgr::writeSimLog(size_t  num ){
  //assert(_simLog != 0 );
  for(size_t  i = 0  ; i< num  ; i++){
    for(size_t j  =0 ; j < inlist.size() ; j++){
        bitset<64> inbit(gatelist[inlist[j]]->_simpattern);
        *_simLog << inbit[i];
    }
    *_simLog << " ";
    for(size_t k  =0 ; k < outlist.size() ; k++){
      bitset<64> outbit(gatelist[outlist[k]->getid()]->_simpattern);
      *_simLog << outbit[i];
    }

    *_simLog<<endl;
  }
}



