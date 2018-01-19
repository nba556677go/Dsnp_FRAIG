/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;
bool sortfano(CirGateV& a  , CirGateV& b ){
  return a.gate()->getid() < b.gate()->getid();
}
bool
readcut
(const string& option, vector<string>& tokens) 
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   return true;
}
bool getname(const string& read, vector<string>& name){
  vector<string> vectmp{"" , ""};
  size_t fend = myStrGetTok(read , vectmp[0]); //if(fend == string::npos) return false; 
  string sym = read.substr(fend+1);
  if(vectmp[0].length() == 1 ) return false;
  if(vectmp[0][0] == 'i' ){
    //string::iterator it = vectmp[0].begin()++;
    name.push_back(vectmp[0].substr(1));
    name.push_back(sym);
    int num;
    if(!myStr2Int(name[0] , num)) return false;
    cirMgr->findinput(num)->setname(name[1]);
  return true;}
  if(vectmp[0][0] == 'o'){
    name.push_back(vectmp[0].substr(1));
    name.push_back(sym);
    int num;
    if(!myStr2Int(name[0] , num)) return false;
    cirMgr->findoutput( num)->setname(name[1]);
    return true;
  }
  return false;
}

CirGate* 
CirMgr::findoutput(int &index) const{ return outlist[index];}
CirGate* 
CirMgr::findinput(int &index) const{ return gatelist[inlist[index]];}
enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
void CirMgr::rebuilddf(){
  
    //construct dfsl ist
    for(size_t i = 0  ; i< gatelist.size() ; i++){
      if(gatelist[i] ==0 )continue;
      gatelist[i]->resetdfflag();
    }
    
    CirGate::_globalRef_s++;
    dflist.clear();
    for(size_t  i =0 ; i< outlist.size() ; i++){
      outlist[i]->dfstraversal(cirMgr->dflist );
    }
    // dfupdate= true; 
  
}
/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
CirGate* 
CirMgr::getGate(int gid) const { 
       if(gid>= gatelist.size()) return 0;
      else return gatelist[gid];
     
}



void
CirMgr::readaigfan(int& num0 , int& num1 , int&num2){
  //cout<<"infan"<<endl;
 // cout<<"fanin1id: "<<num1<<" fanin2id: "<<num2<<endl;
   if(gatelist[num1/2]==0 && gatelist[num2/2]==0){
   //     cout<<"undefined1"<<endl;
        CirGate* c1 = new UndefGate(size_t(0) , UNDEF_GATE , size_t(0), num1/2);
        CirGate* c2 = new UndefGate(size_t(0) , UNDEF_GATE, size_t(0) , num2/2);
        gatelist[num1/2] = c1; gatelist[num2/2] = c2;
       // idlist.push_back(num1/2); idlist.push_back(num2/2);
       
      }
    else if(gatelist[num1/2]==0 && gatelist[num2/2]!=0){
     // cout<<"undefined2"<<endl;
      CirGate* c1 = new UndefGate(size_t(0) , UNDEF_GATE , size_t(0) , num1/2);
      gatelist[num1/2] = c1;//idlist.push_back(num1/2);

    }
    else if(gatelist[num1/2]!=0 && gatelist[num2/2]==0){
      //cout<<"undefined3"<<endl;
      CirGate* c2 = new UndefGate(size_t(0) , UNDEF_GATE , size_t(0) , num2/2);
      gatelist[num2/2] = c2;//idlist.push_back(num2/2);
    }
    else{  }
    //cout<<"settting"<<endl;
     gatelist[num0 /2]->setfanin(CirGateV(gatelist[num1 /2]  , num1 % 2)); 
     gatelist[num0 /2]->setfanin(CirGateV(gatelist[num2 /2] , num2 % 2)); 
     gatelist[num1 /2]->setfanout(CirGateV(gatelist[num0 /2] , num1%2));
     gatelist[num2 /2]->setfanout(CirGateV(gatelist[num0 /2] , num2%2));

   // cout<< num0/2<<"aigfan1 "<<gatelist[num0/2]->getfanindex(0)->gate()<<" aigfan2 "<<gatelist[num0/2]->getfanindex(1)->gate()<<endl;
}


bool
CirMgr::readCircuit(const string& fileName)
{
  ifstream file(fileName);
  if(!file.good()) return false;
    string eachline;
    vector<string> readbf;
    //cirMgr->_linenum = 0;
    while(getline(file , eachline)){
       readbf.push_back(eachline);
      //(cirMgr->_linenum)++;
    }
  
 if(readbf[0][0]==' ') return false;
  for(unsigned i = 0 ; i< readbf.size() ; i++){
    if(readbf[i].size()==1 && (readbf[i][0] =='c'|| readbf[i][0] ==' ')) break;
    
    vector<string> tokens;
    if(!readcut(readbf[i], tokens))return false;
    if(i ==0 ){
      if(tokens[0] != "aag"){ errMsg = "aag"; parseError(MISSING_DEF); return false;}
      if(tokens.size()< 6){errMsg ="M"; parseError(MISSING_NUM); return false;}
      if(tokens.size()>6) return false;
      cirMgr->_total = stoi (tokens[1]); cirMgr->_innum = stoi(tokens[2]); cirMgr->_latnum = stoi(tokens[3]); cirMgr->_outnum = stoi(tokens[4]); cirMgr->_andnum = stoi(tokens[5]);
      cirMgr->gatelist.resize(cirMgr->_total+ cirMgr->_outnum+1 , 0);
       //new const first
     cirMgr->gatelist[0] = new ConstGate(size_t(0), CONST_GATE , 0 , size_t(0));
     //cirMgr->idlist.push_back(0);  

    } 
    


    else if( i <= cirMgr->_innum ){
      if(tokens.size() >1 ) return false;
      int num ;
      if(!myStr2Int(tokens[0] , num)) {errMsg=tokens[0]; parseError(MISSING_NUM); return false;}
      CirGate* input;
      //cout<<"input"<<endl;
        input = new PIGate(size_t(0) , PI_GATE , i+1 , num/2 );
      //else{input = new ConstGate(size_t(0), CONST_GATE , i+1 , size_t(0) );}  
      cirMgr-> gatelist[num/2] = input;
     // cirMgr-> idlist.push_back(input -> getid());
      cirMgr->inlist.push_back(input->getid());
    } 
    else if( i <= (cirMgr->_innum+ cirMgr->_outnum)){
      if(tokens.size() >1 ) return false;
      int num ;
      if(!myStr2Int(tokens[0] , num)) {errMsg=tokens[0]; parseError(MISSING_NUM); return false;}
      //cout<<"output"<<endl;
      CirGate* output = new POGate(0 , PO_GATE , i+1 , cirMgr->_total+ (i-cirMgr->_innum) );
      
      cirMgr-> gatelist[cirMgr->_total+ (i-cirMgr->_innum)] = output;
      //cirMgr-> idlist.push_back(output -> getid());
      cirMgr->outlist.push_back(output);
      output->_iolist.push_back(num);
      
    }
    else if(i <= (cirMgr->_innum+ cirMgr->_outnum+cirMgr->_andnum)){

      if(tokens.size() !=3){errMsg=tokens[0]; parseError(MISSING_NUM); return false;}
      int num ; int io1; int io2;
      /*for(int i = 0 ; i < tokens.size() ; i++){
        if(!myStr2Int(tokens[i] , num[i])) return false;
        cout<<"appear 3 times"<< i <<endl;
      }*/
      if(!myStr2Int(tokens[0] , num)||!myStr2Int(tokens[1] , io1)|| !myStr2Int(tokens[2] , io2)) {errMsg=tokens[0]; parseError(MISSING_NUM); return false;}
      //cout<<"readaig"<<endl;
      //cout<<"num: "<<num<<endl;
       CirGate* aig = new AIGate( 0 , AIG_GATE , i+1 , num/2);//
       cirMgr-> gatelist[num/2] = aig;
       //cout<<"gateget"<<endl;
      // cirMgr-> idlist.push_back(aig -> getid());
       cirMgr->aiglist.push_back(aig->getid());
       aig->_iolist.push_back(io1);
       aig->_iolist.push_back(io2);

    }
  // setname
    else if(i>(cirMgr->_innum+ cirMgr->_outnum+cirMgr->_andnum)){
      
        vector<string> name;
        if(!getname(readbf[i] , name )){ return false;}
        
    }    
  }

  //connect fanin fanout
  /*
  for(unsigned i = (cirMgr->_innum+1) ; i< readbf.size() ; i++){
    if(readbf[i].size()==1 && readbf[i][0] =='c') break;
    vector<string> tokens;
    if(!readcut(readbf[i], tokens))return false;
    //set output
    if(i <= (cirMgr->_innum + cirMgr->_outnum)){
       int num;
      if(!myStr2Int(tokens[0] , num)) return false;
      if(gatelist[num /2] == 0 ) { 
        CirGate* c = new UndefGate(size_t(0) , UNDEF_GATE , size_t(0) , num/2);
        gatelist[num/2] = c;idlist.push_back(num/2);
      }
      //CirGateV* v = 
      gatelist[cirMgr->_total+ (i-cirMgr->_innum)]->setfanin(new CirGateV(gatelist[num/2] , num % 2));
      gatelist[num /2]->setfanout(new CirGateV(gatelist[cirMgr->_total+ (i-cirMgr->_innum)] , num%2));
    }
    else if(i <= (cirMgr->_innum+ cirMgr->_outnum+cirMgr->_andnum)){
      int num0 ; int num1 ; int num2;
      if(!myStr2Int(tokens[0] , num0)||!myStr2Int(tokens[1] , num1)|| !myStr2Int(tokens[2] , num2)) return false;
      cirMgr->readaigfan(num0 , num1 , num2 );
         
    }
  }  
  */

  //new version setfaninout
  for(size_t i =0 ; i< aiglist.size() ; i++){
    //assert(gatelist[aiglist[i]]->getiolist().size()==2);
    for(size_t j =0 ; j< gatelist[aiglist[i]]->getiolist().size() ; j++){
      if(gatelist[gatelist[aiglist[i]]->getioindex(j)/2] == 0){
        CirGate* undef =new UndefGate(size_t(0) , UNDEF_GATE , size_t(0) , gatelist[aiglist[i]]->getioindex(j)/2);
        gatelist[gatelist[aiglist[i]]->getioindex(j)/2] = undef;//idlist.push_back(aiglist[i]->getioindex(j)/2);
       
      }
      gatelist[aiglist[i]]->setfanin(CirGateV(gatelist[gatelist[aiglist[i]]->getioindex(j)/2] , gatelist[aiglist[i]]->getioindex(j) % 2)); 
      gatelist[gatelist[aiglist[i]]->getioindex(j)/2]->setfanout(CirGateV(gatelist[aiglist[i]] , gatelist[aiglist[i]]->getioindex(j)%2)); 
    }
  }

  for(size_t i = 0 ; i< outlist.size() ; i++){
  //	assert(outlist[i]->_iolist.size()==1);
  	if(gatelist[outlist[i]->getioindex(0)/2] == 0){
        CirGate* undef =new UndefGate(size_t(0) , UNDEF_GATE , size_t(0) , outlist[i]->getioindex(0)/2);
        gatelist[outlist[i]->getioindex(0)/2] = undef;//idlist.push_back(outlist[i]->getioindex(0)/2);
     }
     outlist[i]->setfanin( CirGateV(gatelist[outlist[i]->getioindex(0)/2] , outlist[i]->getioindex(0) % 2)); 
     gatelist[outlist[i]->getioindex(0)/2]->setfanout( CirGateV(outlist[i] , outlist[i]->getioindex(0)%2)); 
    
  }

  //sortfano
  
  for(size_t i = 0 ; i< gatelist.size() ; i++){
    if(gatelist[i] ==0 )continue;
   
    std::sort(gatelist[i]->getfanout().begin() , gatelist[i]->getfanout().end(),
    sortfano);
  }

  //construct dfslist
    CirGate::_globalRef_s++;
  
  for(size_t  i =0 ; i< outlist.size() ; i++){
    outlist[i]->dfstraversal(cirMgr->dflist );
  }
  dfupdate = true;
  /*for (size_t i = 0 ; i< dflist.size() ; i++){
    assert(cirMgr->dflist[i]->gettype()!= UNDEF_GATE);
   
  }*/

   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{ cout<<endl;
  cout<< "Circuit Statistics"<<endl;
  cout<<"=================="<<endl;
  //for(int i = 0 ; i < inlist.size(); i ++ ){}
   cout<<"  PI"<<setw(12)<<right<< _innum<<endl;
   cout<<"  PO"<<setw(12)<<right<<_outnum<<endl;
   cout<<"  AIG"<<setw(11)<<right<<_andnum<<endl;
   cout<<"------------------"<<endl;
   cout<<"  Total"<< setw(9)<<right<<_innum+_outnum+_andnum<<endl;  

}


void
CirMgr::printNetlist() const
{
 //assert(dfupdate==true);

  cout<<endl;
  size_t j = 0 ;
  for (size_t i = 0 ; i< dflist.size() ; i++){
    if(dflist[i]->getTypeStr()!="UNDEF"){
      if(dflist[i]->getfanin().size() == 0){
        if(dflist[i]->getTypeStr()!="CONST"){
          cout<<"["<<j<<"] "<<dflist[i]->getTypeStr()<<"  "<< dflist[i]->getid();
          dflist[i]->printname();cout<<endl;
        }
        else cout<<"["<<j<<"] "<<dflist[i]->getTypeStr()<<"0"<<endl;

      }
      else if(dflist[i]->getfanin().size() == 1){
      
        cout<<"["<<j<<"] "<<dflist[i]->getTypeStr()<<"  "<< dflist[i]->getid()<<" ";
        dflist[i]->printgatev(dflist[i]->getfanindex(0) );
        dflist[i]->printname(); cout<<endl;
      }
      else {
        //assert(dflist[i]->getfanin().size() == 2);
        cout<<"["<<j<<"] "<<dflist[i]->getTypeStr()<<" "<< dflist[i]->getid()<<" ";
        dflist[i]->printgatev(dflist[i]->getfanindex(0)); cout<<" "; 
        dflist[i]->printgatev(dflist[i]->getfanindex(1)); 
        dflist[i]->printname(); cout<<endl;
       
      }
      ++j;
    }   
  }
}


void
CirMgr::printPIs() const
{  
   cout << "PIs of the circuit:";
   for(size_t i = 0 ; i< inlist.size() ; i++){
     cout<<" "<<gatelist[inlist[i]]->getid();
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
    for(size_t i = 0 ; i< outlist.size() ; i++){
     cout<<" "<<outlist[i]->getid();
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{ 
  vector<unsigned> flotlist;
  vector<unsigned> notuselist;
  for(size_t i = 1 ;  i< cirMgr->gatelist.size() ; i++){
    if(cirMgr->gatelist[i] == 0) continue;
    if(cirMgr->gatelist[i]->getfanout().size() == 0){
      //cout<<"in1"<<endl;
      if(cirMgr->gatelist[i]->gettype() != PO_GATE){
         notuselist.push_back(gatelist[i]->getid());
       }
    }
    if(cirMgr->gatelist[i]->getfanin().size() !=0 ){ 
      //cout<<"in2"<<endl;
      //assert(i ==1 );
      for(size_t j = 0 ; j < cirMgr->gatelist[i]->getfanin().size() ; j++){
       // cout<<"judgeflot"<<endl;
        //cout<<cirMgr->gatelist[i]->getfanindex(j)->gate()->gettype()<<endl;
        if(cirMgr->gatelist[i]->getfanindex(j).gate()->gettype() == UNDEF_GATE) {
         // cout<<"pushflot"<<endl;
          flotlist.push_back(gatelist[i]->getid()); break;
        }
      }
    }
  }
  //cout<<"float1 "<<flotlist[0]<<endl;
  //printing part
  if(flotlist.size()!=0){
    cout<<"Gates with floating fanin(s):";
    for(size_t i=0 ; i < flotlist.size() ; i++){ cout<<" "<<flotlist[i];}
    cout<<endl;
  }

  if(notuselist.size()!= 0){
    cout<<"Gates defined but not used  :";
    for(size_t i = 0 ; i < notuselist.size() ; i++) cout<<" "<<notuselist[i];
    cout<<endl;
  }

}




void
CirMgr::printFECPairs() const
{ //sortinvert?????????
  //sortfec();
  //vector<vector<CirGate*> > prifecgroup = fecgroup[0];
 
  size_t printnum = 0;
  for(size_t i = 0 ; i< gatelist.size() ; ++i){
    if(gatelist[i] ==0)continue;
    if(gatelist[i]->getTypeStr() == "AIG" || gatelist[i]->getTypeStr() =="CONST" ){
      if(gatelist[i]->getfechead() ==true && gatelist[i]->getsmiupos()!= INT64_MAX){
        cout<<"["<<printnum<<"]";
        for(size_t j =0 ;  j< fecgroup[0][gatelist[i]->getsmiupos()].size() ; ++j){
          cout<<" ";
          if(fecgroup[0][gatelist[i]->getsmiupos()][j]->_simpattern == ~(fecgroup[0][gatelist[i]->getsmiupos()][0]->_simpattern) ){
            cout<<"!";
           }
          cout<<fecgroup[0][gatelist[i]->getsmiupos()][j]->getid();
        }
        printnum++;
        cout<<endl;
      }
    }
    
  }
  /*
  for(size_t i =0 ; i < fecgroup[0].size(); ++i){
    cout<<"["<<i<<"]";
    for(size_t j =0 ;  j< fecgroup[0][i].size() ; ++j){
      cout<<" ";
      if(fecgroup[0][i][j]->_simpattern == ~(fecgroup[0][i][0]->_simpattern) ){
        cout<<"!";
      }
      cout<<fecgroup[0][i][j]->getid();
      //cout<<" inrow: "<<fecgroup[0][i][j]->getsmiupos();
    }
    cout<<endl;
  }*/
}

void
CirMgr::writeAag(ostream& outfile) const
{
  
  size_t dfaigcount =0 ;
  for(int i = 0 ; i < dflist.size() ; i++){
   // assert(dflist[i]!=0);
    if(dflist[i]->gettype() == AIG_GATE){
       ++dfaigcount;
    }
  }

  outfile<<"aag "<<_total<<" "<<_innum<<" "<<_latnum<<" "<<_outnum<<" "<<dfaigcount<<endl;
  for(int i = 0 ; i < inlist.size() ; i++){
    outfile<<2*(gatelist[inlist[i]]->getid())<<endl;
  }
  for(int i = 0 ; i < outlist.size() ; i++){
    //assert(outlist[i]->getfanin().size()==1);
    outfile<<2*(outlist[i]->getfanindex(0).gate()->getid()) + (outlist[i]->getfanindex(0).printinv())<<endl;
  }
  
  //aig
  for(int i = 0 ; i < dflist.size() ; i++){
    //assert(dflist[i]!=0);
    if(dflist[i]->gettype() == AIG_GATE){
      //assert(dflist[i]->getfanin().size()==2);
      outfile<<2*(dflist[i]->getid())<<" "<<2*(dflist[i]->getfanindex(0).gate()->getid()) + (dflist[i]->getfanindex(0).printinv());
      outfile<<" "<<2*(dflist[i]->getfanindex(1).gate()->getid()) + (dflist[i]->getfanindex(1).printinv())<<endl;
    }
  }

  for(int i = 0 ; i < inlist.size() ; i++){
    if(gatelist[inlist[i]]->getname()!="")
    outfile<<"i"<<i<<" "<<gatelist[inlist[i]]->getname()<<endl;
  }
  for(int i = 0 ; i < outlist.size() ; i++){
    if(outlist[i]->getname()!="")
    outfile<<"o"<<i<<" "<<outlist[i]->getname()<<endl;
  }
  outfile<<"c"<<endl;
  /*for(int i = 0 ; i < 1000 ; i++){
    outfile<<"cirg "<< i  << " -fano 1000" <<endl;
  }
  for(int i = 8000 ; i >20 ; i--){
    outfile<<"cirg "<< i  <<" -fani 1000"<<endl;
  }*/


}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{ unsigned innum =0  ; unsigned aignum =0;
  vector <unsigned> ginput;
  vector<unsigned> pionly;
  CirGate::_globalRef_s++;
  g->wrgatetraversal(ginput); 
  //std::sort(ginput.begin() , ginput.end() );
  for(size_t i = 0 ; i< ginput.size(); i++ ){
    if(gatelist[ginput[i]]->getTypeStr() == "AIG"){
      aignum++;
    }
    if(gatelist[ginput[i]]->getTypeStr() == "PI" ){
      //as input
      pionly.push_back(ginput[i]);
      innum++;
    }
  }
  std::sort(pionly.begin() , pionly.end());
  outfile<<"aag "<<g->getid()<<" "<<innum<<" 0 1 "<<aignum<<endl;
  for(size_t i = 0 ; i< pionly.size(); i++ ){
    
    if(gatelist[pionly[i]]->getTypeStr() == "PI" ){
      //as input
      outfile<<2*pionly[i]<<endl;
    }
  }
  cout<<2*g->getid()<<endl;
  for(size_t i = 0 ; i< ginput.size(); i++ ){ 
    if(gatelist[ginput[i]]->getTypeStr() == "AIG"){
      
      outfile<<2*(gatelist[ginput[i]]->getid())<<" "<<2*(gatelist[ginput[i]]->getfanindex(0).gate()->getid()) + (gatelist[ginput[i]]->getfanindex(0).printinv());
      outfile<<" "<<2*(gatelist[ginput[i]]->getfanindex(1).gate()->getid()) + (gatelist[ginput[i]]->getfanindex(1).printinv())<<endl;
      
    }
  }

  for(int i = 0 ; i < ginput.size() ; i++){
    if(gatelist[ginput[i]]->getname()!="")
    outfile<<"i"<<i<<" "<<gatelist[inlist[i]]->getname()<<endl;
  }
  outfile<<"o0 "<< g->getid()<<endl;
  
  outfile<<"c"<<endl;
}





/*void
CirMgr::sortfec(){
  for(size_t  i = 0 ;  i< fecgroup.size() ; i++){
    std::sort(fecgroup[i].begin() , fecgroup[i].end() , fecsort);
  } 
}*/
