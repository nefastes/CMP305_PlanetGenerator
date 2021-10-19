#pragma once
#include <string>
#include <map>
#include "D3D.h"
class LSystem
{
public:
	LSystem(std::string Axiom);

	//Get the string that represents the current state of the L-System
	inline const std::string GetCurrentSystem() { return m_CurrentSystem; }
	inline void SetAxiom(std::string Axiom) { m_Axiom = Axiom; }
	inline const std::string GetAxiom() { return m_Axiom; }


	void AddRule(const char, const std::string&);	//Add a rule to the system
	void Run(const unsigned count);					//Run the system a set number of times
	void Iterate();									//Apply the rules one time
	void Reset();									//Set the system back to its initial state

private:
	std::string m_Axiom;
	std::string m_CurrentSystem;
	std::map<char, std::string> m_rules;
};

