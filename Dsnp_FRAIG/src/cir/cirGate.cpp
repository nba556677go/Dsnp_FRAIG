/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include <bitset>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.
extern CirMgr *cirMgr;
size_t CirGate::_globalRef_s = 1;
// TODO: Implement memeber functions for class(es) in cirGate.h
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
   /*
   if(a!=0){
    convert2dig(a/2 );
    cout<<(a%2);
   }*/
 }
/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{ 
  cout<<"================================================================================"<<endl;
  stringstream s;
  if(_name!= ""){
    cout<<"= "<< getTypeStr()<<"("<<_gateid<<")"<<"\""<<_name<<"\""<<", line "<<_line<<endl;
    //cout<<"= "<<left<<s.str()<<endl;
  }
  else {
    cout<<"= "<<getTypeStr()<<"("<<_gateid<<")"<<", line "<<_line<<endl;
  }
  cout<<"= FECs:";
  
  if(cirMgr->getfecgroup().size()!=0){
  printgatefec();}
    cout<<endl;
 
  cout<<"= Value: ";
  vector<size_t> dig;
  convert2dig(_simpattern);/*
  size_t size = dig.size();
  for(size_t i = 0 ; i < (64 -size) ; i++)
    dig.insert(dig.begin() , 0);
  
  for(size_t i = dig.size() - 1 ; i>= 0 ; i-- ){
    cout<<dig[i];
  }*/
  

  cout<<endl;
  cout<<"================================================================================"<<endl;
}

void
CirGate::printgatefec()const {
  
  if(_smiupos == INT64_MAX) return;
  if(this->getTypeStr()=="PO" ||this->getTypeStr()=="PI" || this->getTypeStr()=="UNDEF"){return;}
  
  //cout<<"_row :"<< _smiupos<<" size: "<<cirMgr->getfecgroup()[_smiupos].size()<<endl;
  for(size_t i = 0 ; i<cirMgr->getfecgroup()[_smiupos].size() ; i++ ){
    
    if(cirMgr->getfecgroup()[_smiupos][i]->getid() == this->getid()) continue;
    cout<<" ";
    if(cirMgr->getfecgroup()[_smiupos][i]->_simpattern == ~(this->_simpattern)){
      cout<<"!";
    }
    cout << cirMgr->getfecgroup()[_smiupos][i]->getid();
  }
  
  
  
  
}

void
CirGate::fanintraversal(int count , int &level ,  bool inv , CirGate* g )const{
  if(count == level+1) return;
  for(int i =0 ; i< count; i++) {cout<<"  ";}
   if(count <= level ){
     stringstream s; 
     if(inv == true) { s<<"!"<<g->getTypeStr() <<" "<<g->getid();}
     else {s<<g->getTypeStr() <<" "<<g->getid(); }
     cout<<s.str(); 
     if(g->isgloblref()&& g->getfanin().size()!=0 && count!=level ) {
      
       cout<<" (*)"<<endl; return;
     }
     cout<<endl;
     if(count!= level){g->setglobal();}
     
      for(size_t i =0 ; i< g->getfanin().size() ; i++){
        //if(!(g->getfanindex(i) ->gate()->isgloblref() )){
           //g->getfanindex(i)->gate()->setglobal();
           fanintraversal( count+1 , level  , g->getfanindex(i).isInv() ,  g->getfanindex(i).gate()  );
        //}
      }
   }
}

void
CirGate::fanoutraversal (int count , int &level ,  bool inv ,CirGate* g )const{
  if(count == level+1) return;
  for(int i =0 ; i< count; i++) {cout<<"  ";}
   if(count <= level ){
     stringstream s; 
     if(inv == true) { s<<"!"<<g->getTypeStr() <<" "<<g->getid();}
     else {s<<g->getTypeStr() <<" "<<g->getid(); }
     cout<<s.str(); 
     if(g->isgloblref()&&g-> getfanout().size()!=0 && count!=level) {
         cout<<" (*)"<<endl; return;
     }
     cout<<endl;
     if(count!= level){g->setglobal();}
      for(int i =0 ; i< g->getfanout().size() ; i++){
        //if(!(g->getfanindex(i) ->gate()->isgloblref() )){
           //g->getfanindex(i)->gate()->setglobal();
           fanoutraversal( count+1 , level  , g->getfanoutindex(i).isInv() ,  g->getfanoutindex(i).gate() );
        //}
      }
   }
}

void
CirGate::reportFanin(int level) const
{ 
   //assert (level >= 0);
   CirGate::_globalRef_s++;
   int count = 0;
   //vector<string> rep;
   (this)->fanintraversal(count , level  , false , const_cast<CirGate*>(this) );
   

}



void
CirGate::reportFanout(int level) 
{ 
   //assert (level >= 0);
   CirGate::_globalRef_s++;
   int count = 0;
   this->fanoutraversal(count , level  , false , const_cast<CirGate*>(this) );
}


 void
 CirGate::dfstraversal(vector<CirGate* > & dflist ){
    //if(_faninList.size() == 0 )return;
   for(int i =0 ; i< _faninList.size() ; i++){
     if(!(_faninList[i].gate()->isgloblref() )){
       _faninList[i].gate()->setglobal();
       _faninList[i].gate()->dfstraversal(dflist );
     }
   }
   cirMgr->getGate(this->getid())->setdflag();
   //if(this->gettype() !=UNDEF_GATE){dflist.push_back(this); }
   dflist.push_back(this);
 }

void
CirGate::wrgatetraversal(vector<unsigned>& wrlist ){
  for(int i =0 ; i< _faninList.size() ; i++){
     if(!(_faninList[i].gate()->isgloblref() )){
       _faninList[i].gate()->setglobal();
       _faninList[i].gate()->wrgatetraversal(wrlist );
     }
   }
  
   //if(this->gettype() !=UNDEF_GATE){dflist.push_back(this); }
   wrlist.push_back(this->getid());
}

 void
 CirGate::rmfani_opti(){
   //assert(_faninList.size()==2);
   for(int i = 0  ;  i < _faninList[0].gate()->getfanout().size() ; i++){
     if(_faninList[0].gate()->getfanoutindex(i).gate()==this ) {
     _faninList[0].gate()->getfanout().erase(_faninList[0].gate()->getfanout().begin()+ i);
      --i;
     }
   }
    for(int i = 0  ;  i < _faninList[1].gate()->getfanout().size() ; i++){
     if(_faninList[1].gate()->getfanoutindex(i).gate()==this ){ 
      _faninList[1].gate()->getfanout().erase(_faninList[1].gate()->getfanout().begin()+ i);
      --i;
    }
   }
   
 }


 bool
 CirGate::findoutgate(CirGate*g){
   for(size_t i  = 0 ; i < _fanoutList.size() ; ++i){
     if(_fanoutList[i].gate() == g)return true;
   }
   return false;
 }


