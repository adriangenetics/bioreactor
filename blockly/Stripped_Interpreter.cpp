#include "Stripped_Interpreter.h"
/*
For my own sanity, here's a stripped-down version of the interpreter. This (probably) isn't going to be turing-complete, but that's okay. It'll have 
everything you need to run one of these devices


Here's some pseudo-code to help plan out what this might look like

//Set temp to 30 after 10 minutes
SLEEP, 10
TEMPSET, 30



//Change temp by 10 every 10 minutes
SETVAR, 0, 0    //Variables are converted to indices
TEMPSET, GETVAR, 0

Label1:
SLEEP, 10
SETVAR, 0, ADD, GETVAR, 0, 10 => SETVAR(0, ADD(GETVAR(0), 10))
TEMPSET, GETVAR, 0
GOTO, Label1    //In final version would just be a byte index



//Toggle shaker every 10 minutes if time is less than 60 minutes
SETSHAKER, 0
SETVAR, 0, 0

Label1:
SLEEP, 10
SETVAR, 0, NOT, GETVAR, 0
SETSHAKER, GETVAR, 0
GOTO_IF, LTHAN, TIME, 60, End
GOTO, Label1
End:


After thinking for a while, I've realized that we really only want conditions, loops, math/booleans, etc. It's going to be very simple
*/

#include <vector>
#include <iostream>

enum class Token : unsigned char
{
	//Logic
	GOTO,
	GOTO_IF,
	GOTO_IFN,

	SLEEP,
	SLEEPD,    //Syntax SLEEPD, expression


	//Math
	ADD,    //ADD, LValue, RValue
	SUB, 
	MUL,
	DIV,

	EQUALS,
	GTHAN,
	LTHAN,


	//Boolean
	AND,
	OR,
	NOT,


	//Variables
	SETVAR,
	GETVAR,


	//Sensing
	TIME,
};

//We don't need to worry about strings and related stuff, we'll just have one float datatype
std::vector<float> Variables;
//Might be a controversial decision, but I'm deciding not to have functions. They can be inlined if I decide to implement them in blockly




//Needs to be implemented in the arduino IDE
void Sleep(unsigned int duration)
{
	std::cout << "Wait " << duration << " ms" << std::endl;
}

unsigned int Time()
{
	//Returns the time in ms since the program started
	return 0;
}



#define GetInt(vect, idx) *(unsigned int*)&vect.data()[idx]
float InterpretToken(std::vector<unsigned char>& Data, unsigned int& CurrentByte)
{
	Token CurrentToken = (Token)Data[CurrentByte];
	++CurrentByte;
	switch (CurrentToken)
	{
	case Token::GOTO:
	{
		unsigned int NewPos = GetInt(Data, CurrentByte);
		CurrentByte = NewPos - 1;
		return 0;
	}
	case Token::GOTO_IF:
		if (InterpretToken(Data, CurrentByte))
		{
			++CurrentByte;
			unsigned int NewPos = GetInt(Data, CurrentByte);
			CurrentByte = NewPos - 1;
			return 0;
		}
		CurrentByte += sizeof(unsigned int);
		return 0;
	case Token::GOTO_IFN:
		if (!InterpretToken(Data, CurrentByte))
		{
			++CurrentByte;
			unsigned int NewPos = GetInt(Data, CurrentByte);
			CurrentByte = NewPos - 1;
			return 0;
		}
		CurrentByte += sizeof(unsigned int);
		return 0;
	case Token::SLEEP:
	{
		unsigned int SleepTime = GetInt(Data, CurrentByte);
		CurrentByte += sizeof(unsigned int) - 1;
		Sleep(SleepTime);
		return 0;
	}
	case Token::SLEEPD:
	{
		float SleepTimeInSeconds = InterpretToken(Data, CurrentByte);
		unsigned int SleepTime = (unsigned int)(SleepTimeInSeconds * 1000);
		Sleep(SleepTime);
		return 0;
	}
	case Token::ADD:
	{
		float LValue, RValue;
		LValue = InterpretToken(Data, CurrentByte);
		++CurrentByte;
		RValue = InterpretToken(Data, CurrentByte);
		return LValue + RValue;
	}
	case Token::SUB:
	{
		float LValue, RValue;
		LValue = InterpretToken(Data, CurrentByte);
		++CurrentByte;
		RValue = InterpretToken(Data, CurrentByte);
		return LValue - RValue;
	}
	case Token::MUL:
	{
		float LValue, RValue;
		LValue = InterpretToken(Data, CurrentByte);
		++CurrentByte;
		RValue = InterpretToken(Data, CurrentByte);
		return LValue * RValue;
	}
	case Token::DIV:
	{
		float LValue, RValue;
		LValue = InterpretToken(Data, CurrentByte);
		++CurrentByte;
		RValue = InterpretToken(Data, CurrentByte);
		return LValue / RValue;
	}
	case Token::EQUALS:
	{
		float LValue, RValue;
		LValue = InterpretToken(Data, CurrentByte);
		++CurrentByte;
		RValue = InterpretToken(Data, CurrentByte);
		return LValue == RValue;
	}
	case Token::GTHAN:
	{
		float LValue, RValue;
		LValue = InterpretToken(Data, CurrentByte);
		++CurrentByte;
		RValue = InterpretToken(Data, CurrentByte);
		return LValue > RValue;
	}
	case Token::LTHAN:
	{
		float LValue, RValue;
		LValue = InterpretToken(Data, CurrentByte);
		++CurrentByte;
		RValue = InterpretToken(Data, CurrentByte);
		return LValue < RValue;
	}
	case Token::AND:
	{
		float LValue, RValue;
		LValue = InterpretToken(Data, CurrentByte);
		++CurrentByte;
		RValue = InterpretToken(Data, CurrentByte);
		return (bool)LValue & (bool)RValue;
	}
	case Token::OR:
	{
		float LValue, RValue;
		LValue = InterpretToken(Data, CurrentByte);
		++CurrentByte;
		RValue = InterpretToken(Data, CurrentByte);
		return (bool)LValue | (bool)RValue;
	}
	case Token::NOT:
	{
		float Value;
		Value = InterpretToken(Data, CurrentByte);
		return !(bool)Value;
	}
	case Token::SETVAR:
	{
		unsigned int VarID = GetInt(Data, CurrentByte);
		CurrentByte += sizeof(int);
		Variables[VarID] = InterpretToken(Data, CurrentByte);
		return 0;
	}
	case Token::GETVAR:
	{
		unsigned int VarID = GetInt(Data, CurrentByte);
		CurrentByte += sizeof(int) - 1;
		return Variables[VarID];
	}
	case Token::TIME:
		--CurrentByte;
		return Time();
	default:
		return 0;
	}
}


void RunProgram(std::vector<unsigned char> Data)
{
	//First, get all variables
	unsigned int NumVars = *(unsigned int*)Data.data();
	unsigned int CurrentByte = sizeof(int);
	Variables.clear();    //Variables are global, so this is just in case
	Variables.reserve(NumVars);

	if (NumVars * sizeof(float) > Data.size())
	{
		//TODO: print error
		return;
	}

	for (int i = 0; i < NumVars; ++i)
	{
		Variables.push_back(*(float*)&Data.data()[CurrentByte]);
		CurrentByte += sizeof(float);    //Using sizeof so this could be ported to 64 bit if ever needed
	}

	//The main loop
	for (CurrentByte; CurrentByte < Data.size(); ++CurrentByte)
	{
		InterpretToken(Data, CurrentByte);
	}
}



#define IntToChars(i) i & 0x000000ff, (i & 0x0000ff00) >> 8, (i & 0x00ff0000) >> 16, (i & 0xff000000) >> 24
#define FloatToChars(f) IntToChars(((float)f))
#define GetByteIdx(NumVars, NumTokens, NumInts) sizeof(int) + NumVars * sizeof(float) + NumTokens * sizeof(Token) + NumInts * sizeof(int)

#ifdef Testing
#include <sstream>
#include <unordered_map>
std::unordered_map<std::string, Token> str2token = {
	{"GOTO", Token::GOTO},
	{"GOTO_IF", Token::GOTO_IF},
	{"GOTO_IFN", Token::GOTO_IFN},

	{"SLEEP", Token::SLEEP},
	{"SLEEPD", Token::SLEEPD},

	{"ADD", Token::ADD},
	{"SUB", Token::SUB},
	{"MUL", Token::MUL},
	{"DIV", Token::DIV},

	{"EQUALS", Token::EQUALS},
	{"GTHAN", Token::GTHAN},
	{"LTHAN", Token::LTHAN},

	{"AND", Token::AND},
	{"OR", Token::OR},
	{"NOT", Token::NOT},

	{"SETVAR", Token::SETVAR},
	{"GETVAR", Token::GETVAR},

	{"TIME", Token::TIME},
};

#define GetByte(Float, idx) ((unsigned char*)&Float)[idx]
#define IsNum(chr) chr == '0' || chr == '1' || chr == '2' || chr == '3' || chr == '4' || chr == '5' || chr == '6' || chr == '7' || chr == '8' || chr == '9'

#include <map>
std::map<std::string, std::vector<int>> Labels;

void InterpretLine(std::vector<unsigned char>& Data, std::stringstream& ss)
{
	std::string Line;
	std::getline(ss, Line, ',');

	if (IsNum(Line[0]))
	{
		float Val = std::stof(Line);
		Data.push_back(GetByte(Val, 0));
		Data.push_back(GetByte(Val, 1));
		Data.push_back(GetByte(Val, 2));
		Data.push_back(GetByte(Val, 3));
	}

	Token CurrentToken = str2token[Line];
	Data.push_back((char)CurrentToken);


	switch (CurrentToken)
	{
	case Token::GOTO:
	{
		//Store the label and the bytes to change once we know what the label points to
		std::getline(ss, Line, ',');
		Labels[Line].push_back(Data.size());
		Data.push_back(0);
		Data.push_back(0);
		Data.push_back(0);
		Data.push_back(0);
		break;
	}
	case Token::GOTO_IF:
	{
		InterpretLine(Data, ss);
		std::getline(ss, Line, ',');
		Labels[Line].push_back(Data.size());
		Data.push_back(0);
		Data.push_back(0);
		Data.push_back(0);
		Data.push_back(0);
		break;
	}
	case Token::GOTO_IFN:
	{
		InterpretLine(Data, ss);
		std::getline(ss, Line, ',');
		Labels[Line].push_back(Data.size());
		Data.push_back(0);
		Data.push_back(0);
		Data.push_back(0);
		Data.push_back(0);
		break;
	}
	case Token::SLEEP:
	{
		std::getline(ss, Line, ',');
		int SleepTime = std::stoi(Line);
		Data.push_back(GetByte(SleepTime, 0));
		Data.push_back(GetByte(SleepTime, 1));
		Data.push_back(GetByte(SleepTime, 2));
		Data.push_back(GetByte(SleepTime, 3));
		break;
	}
	case Token::SLEEPD:
		InterpretLine(Data, ss);
		break;
	case Token::ADD:
	case Token::SUB:
	case Token::MUL:
	case Token::DIV:
	case Token::EQUALS:
	case Token::GTHAN:
	case Token::LTHAN:
	case Token::AND:
	case Token::OR:
	case Token::NOT:
		InterpretLine(Data, ss);    //LValue
		InterpretLine(Data, ss);    //RValue
		break;
	case Token::SETVAR:
	{
		std::getline(ss, Line, ',');
		unsigned int VarIdx = std::stoi(Line);
		Data.push_back(GetByte(VarIdx, 0));
		Data.push_back(GetByte(VarIdx, 1));
		Data.push_back(GetByte(VarIdx, 2));
		Data.push_back(GetByte(VarIdx, 3));
		InterpretLine(Data, ss);
	}
	case Token::GETVAR:
	{
		std::getline(ss, Line, ',');
		unsigned int VarIdx = std::stoi(Line);
		Data.push_back(GetByte(VarIdx, 0));
		Data.push_back(GetByte(VarIdx, 1));
		Data.push_back(GetByte(VarIdx, 2));
		Data.push_back(GetByte(VarIdx, 3));
	}
	case Token::TIME:
		break;
	default:
		break;
	}
}

std::vector<unsigned char> Assemble(std::string Text)
{
	std::vector<std::string> lines;
	std::vector<unsigned char> Data;
	std::stringstream ss(Text);

	std::map<std::string, int> LabelIndices;

	for (std::string line; std::getline(ss, line, '\n');)
		lines.push_back(line);

	int CurrentLine = 0;
	for (CurrentLine; CurrentLine < lines.size(); ++CurrentLine)
	{
		if (lines[CurrentLine] == ".Data")
		{
			++CurrentLine;
			break;
		}
	}
	unsigned int NumVars = 0;
	for (CurrentLine; CurrentLine < lines.size(); ++CurrentLine)
	{
		if (lines[CurrentLine] == "")
			continue;
		if (lines[CurrentLine] == ".Code")
		{
			++CurrentLine;
			break;
		}
		float FloatVal = std::stof(lines[CurrentLine]);
		Data.push_back(GetByte(FloatVal, 0));
		Data.push_back(GetByte(FloatVal, 1));
		Data.push_back(GetByte(FloatVal, 2));
		Data.push_back(GetByte(FloatVal, 3));
		++NumVars;
	}
	Data.emplace(Data.begin(), GetByte(NumVars, 3));    //Push the number of variables into memory
	Data.emplace(Data.begin(), GetByte(NumVars, 2));
	Data.emplace(Data.begin(), GetByte(NumVars, 1));
	Data.emplace(Data.begin(), GetByte(NumVars, 0));


	//Now for the code section. 
	for (CurrentLine; CurrentLine < lines.size(); ++CurrentLine)
	{
		if (lines[CurrentLine] == "")
			continue;
		if (lines[CurrentLine][lines[CurrentLine].size() - 1] == ':')    //If last char is ':', it's probably a label
		{
			LabelIndices[lines[CurrentLine].substr(0, lines[CurrentLine].size() - 1)] = Data.size();
			continue;
		}

		ss = std::stringstream(lines[CurrentLine]);
		InterpretLine(Data, ss);
	}

	for (auto label : LabelIndices)
	{
		std::cout << label.second << std::endl;
		for (int LabelIndex : Labels[label.first])
		{
			Data[LabelIndex] = GetByte(label.second, 0);
			Data[LabelIndex + 1] = GetByte(label.second, 1);
			Data[LabelIndex + 2] = GetByte(label.second, 2);
			Data[LabelIndex + 3] = GetByte(label.second, 3);
		}
	}
	return Data;
}
#endif



//______________________________________________________________________________________________
// THE ENTRY POINT
//______________________________________________________________________________________________
int main()
{
	

	RunProgram(Assemble(
R"(
.Data
1.1

.Code
SLEEP,1000
SLEEP,1000
SLEEP,1000
SLEEPD,GETVAR,0

GOTO,Skip
SLEEP,1000000

Skip:
SLEEP,10
GOTO_IFN,GETVAR,0,ShouldJump

SLEEP,10000

ShouldJump:
SLEEP,10
)"));
}