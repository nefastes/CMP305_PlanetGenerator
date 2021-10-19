#pragma once
#include <string>
#include "D3D.h"
using std::string;
class LSystem
{
public:
	LSystem(string Axiom);

	//Get the string that represents the current state of the L-System
	inline const string GetCurrentSystem() { return m_CurrentSystem; }
	inline void SetAxiom(string Axiom) { m_Axiom = Axiom; }
	inline const string GetAxiom() { return m_Axiom; }


	void AddRule(const char, const string);		//Add a rule to the system
	void Run(const int count);					//Run the system a set number of times
	void Iterate();								//Apply the rules one time
	void Reset();								//Set the system back to its initial state

private:
	string m_Axiom;
	string m_CurrentSystem;
};

