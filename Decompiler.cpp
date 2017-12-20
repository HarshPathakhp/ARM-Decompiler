#include<bits/stdc++.h>

using namespace std;
typedef string (*FnPtr)(vector<string>);
typedef std::unordered_map<std::string, FnPtr> command_map;

const int varSize = 1000;

class Function{
	public:
		string F_name;
		vector<vector<string> > F_body;
};

class Block{	
	public:
	int blockID;
	string Label;
	vector<vector<string> > instructions;
	vector<string> breakPoint;
	vector<int> incomingEdges;
	vector<int> outgoingEdges;
	Block(int blockNo, vector<vector<string> > seqInstructions, string label, vector<string> bP){
		instructions = seqInstructions;
		blockID = blockNo;
		Label = label;
		breakPoint = bP;
	}
	int getFarthestIncomingEdge(){
		if(incomingEdges.size()==0){
			return -1;
		}
		return incomingEdges[incomingEdges.size()-1];
	}
	void addIncomingEdge(int source){
		incomingEdges.push_back(source);
	}
	void addOutgoingEdge(int destination){
		outgoingEdges.push_back(destination);
	}
};

class WhileLoop{
	public:
	WhileLoop(int c){
		startBlock = c;
	}
	int startBlock;
	vector<string> continueConditions;
	vector<int> continueJumps;
	vector<string> breakConditions;
	vector<int> breakJumps;
};

class IfCondition{
	public:
	int startBlock;
	int endBlock;
	string condition;
	IfCondition(int start, int end, string jump){
		startBlock = start;
		endBlock = end;
  		condition = jump;
	}
};

class ControlTransferCommand{
	public:
	int status=0;
	string condition;
	int block;
	string type;
};

// Function prototypes
void InitialiseCommands(command_map &listx);
string removeComments(string);
string removeblanklines(string);
string addSpace(string);
string removeTab(string);
void preProcessFile();
void tokenizeProgram();
vector<string> cleanUpTokens(vector<string>);
void writeTokensToFile();
void runConstructor();
void test();
string mulParser(vector<string>);
string movParser(vector<string>);
string subParser(vector<string>);
string cmpParser(vector<string>);
string swiIntegerIn();
string swiIntegerOut();
string swiCharacterOut();
string swiHandler(vector<string>);
string addParser(vector<string>);
string addsParser(vector<string>);
string subsParser(vector<string>);
string decompileSequentialInstruction(vector<string>,command_map);
void defineVariables();
vector<string> sequentialTranslator(vector<vector<string> >, int);
void dumpCode(vector<string>, string, string);
vector<string> generateControlFlowModel();
void generateLinks();
WhileLoop detectWhileEndPoint(int);
vector<WhileLoop> findWhileLoops();
vector<IfCondition> findIfConditions(); 
void generateControlTransferCommands();
vector<string> GenerateFunctionBody();
string loopTranslator(ControlTransferCommand);
string addsParser(vector<string>);
string subsParser(vector<string>);
string blxParser(vector<string>);
string blParser(vector<string>);
void ClearGlobalStack();
void declarePrototype();
void detectFunctions();
bool checkIfNotWithinLoops(int);

// Global Variables
vector<vector<string> > Program;
int variableCounter = 16; 				// Naming registers as "var"+to_string(variableCounter)
map<string, int> variableTable;			// One to one mapping of registers and variables
vector<Block> callFlowModel;
map<string, int> labelBlock;
vector<WhileLoop> whileLoops;
vector<IfCondition> ifLoops;
int breakJumpPoints[varSize];
ControlTransferCommand jumps[varSize];
int jumpClosing[varSize];
int ifEnding[varSize];
stack<ControlTransferCommand> loopStack;
vector<Function> FunctionArrayVector;
vector<vector<string> > mainBody;
vector<int> returnSkipCount;
map<int,int> excludedIfs;
map<int,int> excludedWhiles;

bool checkIfNotWithinLoops(int q){
	for(int i=0;i<whileLoops.size();i++){
		if(excludedWhiles[i]==1){
			continue;
		}

		if(q>=whileLoops[i].startBlock && q<=whileLoops[i].continueJumps[0]){
			return false;
		}
	}
	for(int i=0;i<ifLoops.size();i++){
		if(excludedIfs[i]==1){
			continue;
		}
		if(q>ifLoops[i].startBlock && q<ifLoops[i].endBlock){
			return false;
		}
	}
	return true;	
}

void findFunctionByName(string name){
	int start = labelBlock[name];
	excludedWhiles.clear();
	excludedIfs.clear();
	for(int i=0;i<whileLoops.size();i++){
		if(i>=whileLoops[i].startBlock && i<=whileLoops[i].continueJumps[0]){
			excludedWhiles[i]=1;
		}
	}
	for(int i=0;i<ifLoops.size();i++){
		if(i>ifLoops[i].startBlock && i<ifLoops[i].endBlock){
			excludedIfs[i]=1;
		}
	}
	returnSkipCount.push_back(0);
	Function f;
	f.F_name = name;
	int flag=0;
	for(int i=start;;i++){
		for(int j=0;j<callFlowModel[i].instructions.size();j++){
			f.F_body.push_back(callFlowModel[i].instructions[j]);
			if(callFlowModel[i].instructions[j][0].compare("mov")==0){
				if(callFlowModel[i].instructions[j][1].compare("pc")==0){
					returnSkipCount[returnSkipCount.size()-1]++;
					if(checkIfNotWithinLoops(i)){
						flag=1;
						break;
					}
				}
			}
		}
		if(flag){
			break;
		}
		if(!(callFlowModel[i].breakPoint[0].compare("empty")==0)){
			f.F_body.push_back(callFlowModel[i].breakPoint);
		}
	}
	FunctionArrayVector.push_back(f);
}
void detectFunctions(){
	vector<string> functionNames;
	for(int i=0;i<Program.size();i++){
		if(Program[i][0].compare("blx")==0 || Program[i][0].compare("bl")==0){
			if(std::find(functionNames.begin(), functionNames.end(), Program[i][1]) == functionNames.end())
				functionNames.push_back(Program[i][1]);
		}
	}
	for(int i=0;i<functionNames.size();i++){
		findFunctionByName(functionNames[i]);
	}
	int fn=0;
	mainBody.clear();
	int i;
	for(i=0;i<Program.size() && fn<functionNames.size();i++){
		while(!Program[i][0].compare("$LABEL$")==0 || !Program[i][1].compare(functionNames[fn])==0){
			mainBody.push_back(Program[i]);
			i++;
		}
		while(returnSkipCount[fn]>0){
			if(Program[i][0].compare("mov")==0){
				if(Program[i][1].compare("pc")==0){
					returnSkipCount[fn]--;
				}
			}
			i++;
		}
		fn++;
	}
	while(i<Program.size()){
		mainBody.push_back(Program[i]);
		i++;
	}
}
void ClearGlobalStack(){
	callFlowModel.clear();
	labelBlock.clear();
	whileLoops.clear();
	ifLoops.clear();
	memset(breakJumpPoints, 0, sizeof(breakJumpPoints));
	memset(jumpClosing, 0, sizeof(jumpClosing));
	memset(ifEnding, 0, sizeof(ifEnding));
	while(!loopStack.empty()){
		loopStack.pop();
	}
	for(int i=0; i<varSize;i++){
		ControlTransferCommand temp;
		jumps[i] = temp;
	}
}
bool checkIfElse(int q){
	for(int i=0;i<ifLoops.size();i++){
		if(q == ifLoops[i].startBlock){
			continue;
		}
		if(ifLoops[i].endBlock == q+1){
			return true;
		}
	}
	return false;
}
vector<string> GenerateFunctionBody(){
	vector<string> body;
	for(int i=0;i<callFlowModel.size();i++){
		vector<string> sequentialInstructions = sequentialTranslator(callFlowModel[i].instructions, loopStack.size());
		for(int j=0;j<sequentialInstructions.size();j++){
			body.push_back(sequentialInstructions[j]);
		}
		for(int j=jumpClosing[i];jumpClosing[i]!=0;jumpClosing[i]--){
			ControlTransferCommand temp = loopStack.top();
			loopStack.pop();
			if(temp.type.compare("while")==0){
				body.push_back(loopTranslator(temp));
			}
			else{
				body.push_back(string(loopStack.size(),'\t')+"}");
			}
		}
		if(jumps[i].status==1){
			if(jumps[i].type.compare("while")==0){
				body.push_back(string(loopStack.size(),'\t')+"do{");
			}
			else{
				if(jumps[i].type.compare("if")==0 && checkIfElse(i)){
					for(int j=jumpClosing[i+1];jumpClosing[i+1]!=0;jumpClosing[i+1]--){
						loopStack.pop();
						body.push_back(string(loopStack.size(),'\t')+"}");
					}
					i++;
					body.push_back(string(loopStack.size(),'\t')+"else{");
					loopStack.push(jumps[i]);
					continue;
				}
				else{
					body.push_back(loopTranslator(jumps[i]));
				}
			}
			if(jumps[i].type.compare("break")!=0 && jumps[i].type.compare("continue")!=0){
				loopStack.push(jumps[i]);
			}
		}
	}
	return body;
}

void generateControlTransferCommands(){
	for(int i=0;i<whileLoops.size();i++){
		ControlTransferCommand temp;
		temp.type = "while";
		temp.status=1;
		temp.block = whileLoops[i].startBlock-1;
		temp.condition = whileLoops[i].continueConditions[0];
		jumps[whileLoops[i].startBlock-1] = temp;
		jumpClosing[whileLoops[i].continueJumps[0]]++;
		for(int j=1;j<whileLoops[i].continueJumps.size();j++){
			ControlTransferCommand temp2;
			temp2.status=1;
			temp2.block = whileLoops[i].continueJumps[j];
			temp2.type = "continue";
			temp2.condition = whileLoops[i].continueConditions[j];
			jumps[whileLoops[i].continueJumps[j]] = temp2;
		}
		for(int j=0;j<whileLoops[i].breakJumps.size();j++){
			ControlTransferCommand temp2;
			temp2.status=1;
			temp2.block = whileLoops[i].breakJumps[j];
			temp2.type = "break";
			temp2.condition = whileLoops[i].breakConditions[j];
			jumps[whileLoops[i].breakJumps[j]] = temp2;
		}
	}
	for(int i=0;i<ifLoops.size();i++){
		ControlTransferCommand temp2;
		if(jumps[ifLoops[i].startBlock].status==1){
			continue;
		}
		temp2.status=1;
		temp2.block = ifLoops[i].startBlock;
		temp2.type = "if";
		temp2.condition = ifLoops[i].condition;
		jumps[ifLoops[i].startBlock] = temp2;
		jumpClosing[ifLoops[i].endBlock]++;
	}
}

vector<IfCondition> findIfConditions(){
	vector<IfCondition> temp;
	for(int i=0;i<callFlowModel.size();i++){
		if(callFlowModel[i].breakPoint[0].at(0) == 'b'){
			if(labelBlock[callFlowModel[i].breakPoint[1]]-1>=i && breakJumpPoints[i]==0){
				ifEnding[labelBlock[callFlowModel[i].breakPoint[1]]-1]++;
				IfCondition newIf(i, labelBlock[callFlowModel[i].breakPoint[1]]-1, callFlowModel[i].breakPoint[0]);
				temp.push_back(newIf);
			}
		}
	}
	return temp;
}

WhileLoop detectWhileEndPoint(int c){
	Block b= callFlowModel[c];
	WhileLoop temp(c);
	int i = b.incomingEdges.size()-1;
	while(b.incomingEdges[i]>=c){
		temp.continueJumps.push_back(b.incomingEdges[i]);
		temp.continueConditions.push_back(callFlowModel[b.incomingEdges[i]].breakPoint[0]);
		i--;
	}
	int lastBlock = temp.continueJumps[0];
	i = lastBlock;
	while(i>=c){
		if(labelBlock[callFlowModel[i].breakPoint[1]] == lastBlock+2 && callFlowModel[lastBlock+1].instructions.size()==0){
			temp.breakJumps.push_back(i);
      		breakJumpPoints[i]=1;
			temp.breakConditions.push_back(callFlowModel[i].breakPoint[0]);
		}
		i--;
	}
	return temp;
}

vector<WhileLoop> findWhileLoops(){
	vector<WhileLoop> temp;
	for(int i=0;i<callFlowModel.size();i++){
		if(callFlowModel[i].Label.compare("")!=0){
			if(callFlowModel[i].getFarthestIncomingEdge()>=i){
				temp.push_back(detectWhileEndPoint(i));
			}
		}
	}
	return temp;
}

string loopTranslator(ControlTransferCommand c)
{
	string command=std::string(loopStack.size(),'\t');
	if(c.type.compare("while")==0){
		if(c.condition.compare("bgt")==0){
			command+="} while(compareRegister > 0);";
		}
		else if(c.condition.compare("ble")==0){
			command+="} while(compareRegister <= 0);";
		}
		else if(c.condition.compare("blt")==0){
			command+="} while(compareRegister < 0);";
		}
		else if(c.condition.compare("bge")==0){
			command+="} while(compareRegister >= 0);";
		}
		else if(c.condition.compare("bne")==0){
			command+="} while(compareRegister != 0);";
		}
		else if(c.condition.compare("beq")==0){
			command+="} while(compareRegister == 0);";
		}
		else if(c.condition.compare("b")==0){
			command+="} while(1);";
		}
	}
	else if(c.type.compare("continue")==0){
		if(c.condition.compare("bgt")==0){
			command+="if(compareRegister > 0){ \n\tcontinue; \n}";
		}
		else if(c.condition.compare("ble")==0){
			command+="if(compareRegister <= 0){ \n\tcontinue; \n}";
		}
		else if(c.condition.compare("blt")==0){
			command+="if(compareRegister < 0){ \n\tcontinue; \n}";
		}
		else if(c.condition.compare("bge")==0){
			command+="if(compareRegister >= 0){ \n\tcontinue; \n}";
		}
		else if(c.condition.compare("bne")==0){
			command+="if(compareRegister != 0){ \n\tcontinue; \n}";
		}
		else if(c.condition.compare("beq")==0){
			command+="if(compareRegister == 0){ \n\tcontinue; \n}";
		}
		else if(c.condition.compare("b")==0){
			command+="if(1){ \n\tcontinue; \n}";
		}
	}
	else if(c.type.compare("break")==0){
		if(c.condition.compare("bgt")==0){
			command+="if(compareRegister > 0){ \n\tbreak; \n}";
		}
		else if(c.condition.compare("ble")==0){
			command+="if(compareRegister <= 0){ \n\tbreak; \n}";
		}
		else if(c.condition.compare("blt")==0){
			command+="if(compareRegister < 0){ \n\tbreak; \n}";
		}
		else if(c.condition.compare("bge")==0){
			command+="if(compareRegister >= 0){ \n\tbreak; \n}";
		}
		else if(c.condition.compare("bne")==0){
			command+="if(compareRegister != 0){ \n\tbreak; \n}";
		}
		else if(c.condition.compare("beq")==0){
			command+="if(compareRegister == 0){ \n\tbreak; \n}";
		}
		else if(c.condition.compare("b")==0){
			command+="if(1){ \n\tbreak; \n}";
		}
	
	}
	else if(c.type.compare("if")==0){				// Inverted on purpose
		if(c.condition.compare("bgt")==0){
			command+="if(compareRegister <= 0){";
		}
		else if(c.condition.compare("ble")==0){
			command+="if(compareRegister > 0){";
		}
		else if(c.condition.compare("blt")==0){
			command+="if(compareRegister >= 0){";
		}
		else if(c.condition.compare("bge")==0){
			command+="if(compareRegister < 0){";
		}
		else if(c.condition.compare("bne")==0){
			command+="if(compareRegister == 0){";
		}
		else if(c.condition.compare("beq")==0){
			command+="if(compareRegister != 0){";
		}
		else if(c.condition.compare("b")==0){
			command+="if(0){";
		}
	}
	return command;

}

void generateLinks(){
	int i=0;
	int blockID = 0;
	while(i<Program.size()){
		while(i<Program.size() && !(Program[i][0].compare("$LABEL$")==0) && (!(Program[i][0].at(0) == 'b') || (Program[i][0].compare("blx")==0) || (Program[i][0].compare("bl")==0))){
			i++;
		}
	    if(i<Program.size() && (Program[i][0].at(0) == 'b')){
			string targetLabel = Program[i][1];
			int targetBlock = labelBlock[targetLabel];
			callFlowModel[blockID].addOutgoingEdge(targetBlock);
			callFlowModel[targetBlock].addIncomingEdge(blockID);
	    }
		blockID++;
		i++;
	}
}
vector<string> generateCallFlowModel(){
	ClearGlobalStack();
	ControlTransferCommand main;
	loopStack.push(main);
	memset(jumpClosing,0,sizeof(jumpClosing));
	memset(breakJumpPoints,0,sizeof(breakJumpPoints));
	memset(ifEnding,0,sizeof(ifEnding));
	int i=0;
	int blockID = 0;
	string currentLabel = "";
	while(i<Program.size()){
		vector<vector<string> > temp;
		while(i<Program.size() && !(Program[i][0].compare("$LABEL$")==0) && (!(Program[i][0].at(0) == 'b') || (Program[i][0].compare("blx")==0) || (Program[i][0].compare("bl")==0))){
			temp.push_back(Program[i]);
			i++;
		} 
	    if(i<Program.size() && Program[i][0].compare("$LABEL$")==0){
	    	labelBlock[Program[i][1]] = blockID+1;  
	    }
	    vector<string> bP;
		if(i == Program.size() && (Program[i-1][0].at(0) == 'b') && (Program[i-1][0].compare("blx") != 0) && (Program[i-1][0].compare("bl") != 0)){
			bP = Program[i-1];
		}
		else if(i<Program.size()){
			bP = Program[i];
		}
		else{
			bP.push_back("empty");
		}
		Block b(blockID, temp, currentLabel,bP);
		callFlowModel.push_back(b);   
		blockID++;
		if(i<Program.size() && Program[i][0].compare("$LABEL$")==0){
			currentLabel = Program[i][1];
		}
		else{
			currentLabel = "";
		}
		i++;  
	}  
	vector<vector<string> > temp;
	vector<string> bp;
	bp.push_back("empty");
	Block b(blockID, temp, currentLabel, bp);
	callFlowModel.push_back(b);
	generateLinks();
	whileLoops = findWhileLoops();
	ifLoops = findIfConditions();
	generateControlTransferCommands();
	vector<string> body = GenerateFunctionBody();
	return body;
}

vector<string> sequentialTranslator(vector<vector<string> > pgm, int tabs){
	command_map listx;
	InitialiseCommands(listx);
	vector<string> temp;
	for(int i=0;i<pgm.size();i++){
		string prefix = string(tabs, '\t');
		prefix = prefix+decompileSequentialInstruction(pgm[i],listx);
		temp.push_back(prefix);
	}
	return temp;
}
void dumpCode(vector<string> temp, string function, string signature){
	ofstream fout;
	fout.open("output.c",ios::app);
	fout<<function<<"{"<<endl;
	for(int i=0;i<temp.size();i++){
		string code = temp[i];
		vector<string> splitCode;
		stringstream ss(code);
		string to;
		while(std::getline(ss,to,'\n')){
			splitCode.push_back(to);
		}
		int j=0;
		string prefix = "";
		while(splitCode[0].at(j)=='\t'){
			prefix += '\t';
			j++;
		}
		fout<<splitCode[0]<<endl;
		for(int j=1;j<splitCode.size();j++){
			fout<<prefix+splitCode[j]<<endl;
		}
	}
	if(!signature.compare("")==0)
		fout<<string(loopStack.size(),'\t')+signature<<endl;
	fout<<"}"<<endl;
	fout<<endl;
	fout.close();
}
string swiIntegerIn(){
 	return "if(var1 == 0){\n\tscanf(\"%d\",&var1);\n}";
}
string swiHandler(vector<string> instruction){
	if(instruction[0].compare("swi")!=0){
		cout<<"Cannot handle using swiHandler"<<endl;
	}
	if(instruction[1].compare("0x00")==0 || instruction[1].compare("0x0")==0){
	return swiCharacterOut();
	}
	else if(instruction[1].compare("0x6b")==0){
		return swiIntegerOut();
	}
	else if(instruction[1].compare("0x6c")==0){
		return swiIntegerIn();
	}
	else if(instruction[1].compare("0x11")==0){
		return "exit(0);";
	}
	else{
		cout<<"Error occured while decompiling SWI operation"<<endl;
		return "";
	}
}
void defineVariables(){
	// Default program variables
	ofstream fout;
	fout.open("output.c");
	fout<<"// Headers"<<endl;
	fout<<"#include<stdio.h>"<<endl;
	fout<<"#include<math.h>"<<endl;
	fout<<"#include<stdlib.h>"<<endl;
	fout<<endl;
	fout<<"// Program Variables"<<endl;
	fout<<"int compareRegister = 0;"<<endl;
	for(int i=1; i<variableCounter; i++){
		fout<<"int var"<<i<<" = 0;"<<endl;
	}
	fout<<endl;
	fout<<"// Program begins"<<endl;
	fout.close();
}

void runConstructor(){
	variableTable["r0"] = 1;
	variableTable["r1"] = 2;
	variableTable["r2"] = 3;
	variableTable["r3"] = 4;
	variableTable["r4"] = 5;
	variableTable["r5"] = 6;
	variableTable["r6"] = 7;
	variableTable["r7"] = 8;
	variableTable["r8"] = 9;
	variableTable["r9"] = 10;
	variableTable["r10"] = 11;
	variableTable["r11"] = 12;
	variableTable["r12"] =13;
	variableTable["r13"] = 14;
	variableTable["r14"] = 15;
}
string removeComments(string prgm)
{
    int n = prgm.length();
    string res;

    // Flags to indicate that single line and multpile line comments
    // have started or not.
    bool cmt = false;

    // Traverse the given program
    for (int i=0; i<n; i++){
        // If single line comment flag is on, then check for end of it
        if (cmt == true && prgm[i] == '\n'){
            cmt = false;
            res+=prgm[i];
 		}
        // If this character is in a comment, ignore it
        else if (cmt)
            continue;

        // Check for beginning of comments and set the approproate flags
        else if (prgm[i] == '@')
            cmt = true;

        // If current character is a non-comment character, append it to res
        else{
        	res += prgm[i];
        	if(prgm[i] == ':'){          // remove multiple instructions from same line
        		res += '\n';
        	}
    	}
    }
    return res;
}

string removeblanklines(string prgm)
{
	int n = prgm.length();
    string res;

    // Traverse the given program
    for (int i=0; i<n; i++){
    	if(prgm[i]=='\n' && prgm[i]==prgm[i-1]){
    		continue;
    	}
    	else{
    		res+=prgm[i];
    	}
    }
    return res;
}

string addSpace(string prgm){
	int n=prgm.length();
	string res;

	for(int i=0;i<n;i++){
		res+=prgm[i];
		if(prgm[i]==','){
			res+=" ";
		}
	}
	return res;
}

string removeTab(string prgm){
	int n=prgm.length();
	string res;
	for(int i=0;i<n;i++){
		if(prgm[i]=='\t'){
			res+=" ";
		}
		else{
			res+=prgm[i];
		}
	}
	return res;
}


void preProcessFile(){
	ifstream fin;
	string s;
	cout<<"Enter the name of the file \t";
	cin>>s;
	s=s+".s";

	fin.open(s);
	string line;
	string str;
	while(getline(fin,line)){
		if(!line.empty()){
			str+=line;
			str+='\n';
		}
	}

	str=removeComments(str);
	str=removeTab(str);
	str=addSpace(str);


	// remove multiple spaces
	size_t pos;
	while( ( pos = str.find( "  " ) )!=std::string::npos )
    	str = str.replace( pos, 2, " " );

	// remove extra blank lines
	str=removeblanklines(str);

	ofstream fout;
	fout.open("output.c");
	fout<<str<<endl;
	fin.close();
	fout.close();
}

vector<string> cleanUpTokens(vector<string> instruction){
	for(int i=0;i<instruction.size();i++){							// Iterating over the tokens
		instruction[i].erase(remove(instruction[i].begin(), instruction[i].end(), ' '), instruction[i].end());	// Remove white spaces
		instruction[i].erase(remove(instruction[i].begin(), instruction[i].end(), ','), instruction[i].end());	// Remove commas
		instruction[i].erase(remove(instruction[i].begin(), instruction[i].end(), '\t'), instruction[i].end());	// Remove tabs
		instruction[i].erase(remove(instruction[i].begin(), instruction[i].end(), '\n'), instruction[i].end());	// Remove new lines
		transform(instruction[i].begin(), instruction[i].end(), instruction[i].begin(), ::tolower);
	}
	for(int i=instruction.size()-1;i>=0;i--){						// Remove empty tokens
		if(instruction[i].empty()){
			instruction.erase(instruction.begin()+i);
		}
	}
	if(instruction.size()==1){										// Marking labels
		if(instruction[0][instruction[0].length()-1]==':'){
			string labelHeader = "$LABEL$";
			instruction.insert(instruction.begin(),labelHeader);
			instruction[1].erase(remove(instruction[1].begin(), instruction[1].end(), ':'), instruction[1].end());
		}
	}
	return instruction;
}

void writeTokensToFile(){
	ofstream fout;
	fout.open("output.c");
	for(int i=Program.size()-1;i>=0;i--){
		if(Program[i].size()==0){
			Program.erase(Program.begin()+i);
		}
	}
	for(int i=0;i<Program.size();i++){
		for(int j=0;j<Program[i].size();j++){
			fout<<Program[i][j]<<";";
		}
		fout<<endl;
	}
	fout.close();
}

void tokenizeProgram(){
	string line;
	ifstream fin;
	fin.open("output.c");

	// Reading file with space as delimiter
	while(getline(fin, line))
	{
		stringstream ss(line);
		istream_iterator<string> begin(ss);
		istream_iterator<string> end;
		vector<string> splitLine(begin, end);		// Splitting instruction by space
		splitLine = cleanUpTokens(splitLine);		// cleaning up extra spaces, tabs, newlines and commas
		Program.push_back(splitLine);
	}
	writeTokensToFile();
}
string mulParser(vector<string> instruction){
	if(instruction[0].compare("mul")!=0){
		cout<<"Error in mulParser"<<endl;
	}
	string translatedCommand = "";
	string targetRegister = instruction[1];
	string sourceRegister1 = instruction[2];
	string sourceRegister2 = instruction[3];
	string targetTransform = "var"+to_string(variableTable[targetRegister]);
	string sourceTransform1, sourceTransform2;
	if(sourceRegister1.at(0)!='#'){
		sourceTransform1 = "var"+to_string(variableTable[sourceRegister1]);
	}
	else{
		sourceTransform1 = sourceRegister1.substr(1);
	}
	if(sourceRegister2.at(0)!='#'){
		sourceTransform2 = "var"+to_string(variableTable[sourceRegister2]);
	}
	else{
		sourceTransform2 = sourceRegister2.substr(1);
	}
	translatedCommand = targetTransform+" = "+sourceTransform1+" * "+sourceTransform2+";";
	return translatedCommand;
}

string subParser(vector<string> instruction){
	if(instruction[0].compare("sub")!=0){
		cout<<"Error in subParser"<<endl;
	}
	string translatedCommand =  "";
	string targetRegister = instruction[1];
	string sourceRegister1 = instruction[2];
	string sourceRegister2 = instruction[3];
	string targetTransform = "var" + to_string(variableTable[targetRegister]);
	string sourceTransform1,sourceTransform2;
	if(sourceRegister1.at(0)!='#'){
		sourceTransform1 ="var"+ to_string(variableTable[sourceRegister1]);
	}
	else{
		sourceTransform1 = sourceRegister1.substr(1);
	}
	if(sourceRegister2.at(0)!='#'){
		sourceTransform2 ="var" + to_string(variableTable[sourceRegister2]);
	}
	else{
		sourceTransform2 = sourceRegister2.substr(1);
	}
	translatedCommand = targetTransform+" = "+sourceTransform1+" - "+sourceTransform2+";";
	return translatedCommand;
}

string cmpParser(vector<string> instruction)
{
	if(instruction[0].compare("cmp")!=0){
		cout<<"Error in cmpParser"<<endl;
	}
	string translatedCommand =  "";
	string sourceRegister1 = instruction[1];
	string sourceRegister2 = instruction[2];
	string targetTransform = "compareRegister";
	string sourceTransform1,sourceTransform2;
	if(sourceRegister1.at(0)!='#'){
		sourceTransform1 ="var"+ to_string(variableTable[sourceRegister1]);
	}
	else{
		sourceTransform1 = sourceRegister1.substr(1);
	}
	if(sourceRegister2.at(0)!='#'){
		sourceTransform2 ="var" + to_string(variableTable[sourceRegister2]);
	}
	else{
		sourceTransform2 = sourceRegister2.substr(1);
	}
	translatedCommand = targetTransform+" = "+sourceTransform1+" - "+sourceTransform2+";";
	return translatedCommand;
}


string movParser(vector<string> instruction)
{
	if(instruction[0].compare("mov")!=0){
		cout<<"Error in movParser"<<endl;
	}
	string translatedCommand =  "";
	if(instruction[1].compare("pc")==0){
		return "return;";
	}
	string targetRegister = instruction[1];
	string sourceRegister1 = instruction[2];
	string targetTransform = "var" + to_string(variableTable[targetRegister]);
	string sourceTransform1;
	if(sourceRegister1.at(0)!='#'){
		sourceTransform1 ="var"+ to_string(variableTable[sourceRegister1]);
	}
	else{
		if(instruction.size()>=4){
			sourceTransform1 = "' '";
		}
		else{
			sourceTransform1 = sourceRegister1.substr(1);
		}
	}
	translatedCommand = targetTransform+" = "+sourceTransform1+";";
	return translatedCommand;
}

string swiIntegerOut(){
 	string translatedCommand = "";
 	translatedCommand= "if(var1==1){ \n \tprintf(\"%d\",var2); \n} ";
 	return translatedCommand;
}

string swiCharacterOut(){
	string translatedCommand = "printf(\"%c\" ,var1); \n";
	return translatedCommand;
}

string addParser(vector<string> instruction){
	if(instruction.at(0).compare("add")!=0){
		cout<<"Error in addParser"<<endl;
	}
	string translatedcommand = "";
	string Tregister=instruction.at(1);
	string S1register = instruction.at(2);
	string S2register = instruction.at(3);
	string targetTransform = "var" +to_string(variableTable[Tregister]);
	string sourceTransform1,sourceTransform2;
	if(S1register.at(0)!='#'){
		sourceTransform1 = "var" + to_string(variableTable[S1register]);
	}
	else{
		sourceTransform1 = S1register.substr(1);
	}
	if(S2register.at(0)!='#'){
		sourceTransform2 = "var" + to_string(variableTable[S2register]);
	}
	else{
		sourceTransform2 = S2register.substr(1);
	}
	translatedcommand = targetTransform + " = "+sourceTransform1 +" + " + sourceTransform2 + ";";
	return translatedcommand;
}
string subsParser(vector<string> instruction){
	if(instruction[0].compare("subs")!=0){
		cout<<"Error in subsParser"<<endl;
	}
	string translatedCommand =  "";
	string targetRegister = instruction[1];
	string sourceRegister1 = instruction[2];
	string sourceRegister2 = instruction[3];
	string targetTransform = "var" + to_string(variableTable[targetRegister]);
	string sourceTransform1,sourceTransform2;
	if(sourceRegister1.at(0)!='#'){
		sourceTransform1 ="var"+ to_string(variableTable[sourceRegister1]);
	}
	else{
		sourceTransform1 = sourceRegister1.substr(1);
	}
	if(sourceRegister2.at(0)!='#'){
		sourceTransform2 ="var" + to_string(variableTable[sourceRegister2]);
	}
	else{
		sourceTransform2 = sourceRegister2.substr(1);
	}
	translatedCommand = targetTransform+" = "+sourceTransform1+" - "+sourceTransform2+";"+"\n"+"compareRegister = "+sourceTransform1+" - "+sourceTransform2+";";
	return translatedCommand;
}
string addsParser(vector<string> instruction){
	if(instruction.at(0).compare("adds")!=0){
		cout<<"Error in addsParser"<<endl;
	}
	string translatedcommand = "";
	string Tregister=instruction.at(1);
	string S1register = instruction.at(2);
	string S2register = instruction.at(3);
	string targetTransform = "var" +to_string(variableTable[Tregister]);
	string sourceTransform1,sourceTransform2;
	if(S1register.at(0)!='#'){
		sourceTransform1 = "var" + to_string(variableTable[S1register]);
	}
	else{
		sourceTransform1 = S1register.substr(1);
	}
	if(S2register.at(0)!='#'){
		sourceTransform2 = "var" + to_string(variableTable[S2register]);
	}
	else{
		sourceTransform2 = S2register.substr(1);
	}
	translatedcommand = targetTransform + " = "+sourceTransform1 +" + " + sourceTransform2 + ";"+ "\n"+"compareRegister = "+sourceTransform1 +" + "+ sourceTransform2+";";
	return translatedcommand;
}
string blxParser(vector<string> instruction){
	if(instruction.at(0).compare("blx")!=0){
		cout<<"Error in blxParser"<<endl;
	}
	string translatedcommand = "";
	translatedcommand = instruction.at(1);
	translatedcommand+="();";
	return translatedcommand;
}	
string blParser(vector<string> instruction){
	if(instruction.at(0).compare("bl")!=0){
		cout<<"Error in blParser"<<endl;
	}
	string translatedcommand = "";
	translatedcommand = instruction.at(1);
	translatedcommand+="();";
	return translatedcommand;
}
void InitialiseCommands(command_map &listx){
    listx.emplace("add",&addParser);
    listx.emplace("sub",&subParser);
    listx.emplace("mul",&mulParser);
    listx.emplace("cmp",&cmpParser);
    listx.emplace("mov",&movParser);
    listx.emplace("swi",&swiHandler);
    listx.emplace("adds",&addsParser);
    listx.emplace("subs",&subsParser);
    listx.emplace("blx",&blxParser);
    listx.emplace("bl",&blParser);
}

string decompileSequentialInstruction(vector<string> instruction,command_map listx){
    return listx.at(instruction.at(0))(instruction);
}
void test(){

	// MulParser Test
	cout<<"mulParser() testing..."<<endl;
	cout<<"Input string: mul r2,r2,#11"<<endl;
	vector<string> temp;
	temp.push_back("mul");
	temp.push_back("r2");
	temp.push_back("r2");
	temp.push_back("#11");
	cout<<"Output: "<<mulParser(temp)<<endl;
	cout<<endl;
	// MulParser Test ends

	//testing subParser
	cout<<"subParser() testing..."<<endl;
	cout<<"Input string: sub r2,r2,#11"<<endl;
	vector<string> temp1;
	temp1.push_back("sub");
	temp1.push_back("r2");
	temp1.push_back("r2");
	temp1.push_back("#11");
	cout<<"Output: "<<subParser(temp1)<<endl;
	cout<<endl;
	// subParser test ends

	//cmpParser test begins
	cout<<"cmpParser() testing..."<<endl;
	cout<<"Input string: cmp r2,#11"<<endl;
	vector<string> temp2;
	temp2.push_back("cmp");
	temp2.push_back("r2");
	temp2.push_back("#11");
	cout<<"Output: "<<cmpParser(temp2)<<endl;
	cout<<endl;
	// cmpParser test ends

	// movParser test begins
	cout<<"movParser() testing..."<<endl;
	cout<<"Input string: mov r2,#11"<<endl;
	vector<string> temp3;
	temp3.push_back("mov");
	temp3.push_back("r2");
	temp3.push_back("#11");
	cout<<"Output: "<<movParser(temp3)<<endl;
	cout<<endl;
	// mov parser test ends

	// addParser test
	cout<<"addParser() testing..."<<endl;
	cout<<"Input string: add r2,r2,#11"<<endl;
	vector<string> temp4;
	temp4.push_back("add");
	temp4.push_back("r2");
	temp4.push_back("r2");
	temp4.push_back("#11");
	cout<<"Output: "<<addParser(temp4)<<endl;
	cout<<endl;
	// add parser test ends

	// blxParser test
	cout<<"blxParser() testing..."<<endl;
	cout<<"Input string: blx loop"<<endl;
	vector<string> temp5;
	temp5.push_back("blx");
	temp5.push_back("loop");
	cout<<"Output: "<<blxParser(temp5)<<endl;
	cout<<endl;
	// blx parser test ends

}

void declarePrototype(string name){
	ofstream fout;
	fout.open("output.c",ios::app);
	fout<<"void "+name+"();"<<endl;
	fout.close();
}

int main()
{
	runConstructor();				// Defining program variables
	preProcessFile();				// Phase 1 - Implemented by Rahul Garg
	tokenizeProgram();				// Phase 2 - Implemented by Nihesh Anderson
	defineVariables();				// Declares variables
	mainBody = Program;
	generateCallFlowModel();
	detectFunctions();
	for(int i=0;i<FunctionArrayVector.size();i++){
		declarePrototype(FunctionArrayVector[i].F_name);
	}
	ofstream fout;
	fout.open("output.c",ios::app);
	fout<<endl;
	fout.close();
	for(int i=0;i<FunctionArrayVector.size();i++){
		Program = FunctionArrayVector[i].F_body;
		dumpCode(generateCallFlowModel(),"void "+FunctionArrayVector[i].F_name+"()","");
	}
	Program = mainBody;
	dumpCode(generateCallFlowModel(),"int main()","return 0;");  	// Decompiles code and writes to file
	// test();					// Write your tests here
	return 0;
}
