#include "LSystem.h"

LSystem::LSystem(string Axiom) :
	m_Axiom(Axiom), m_CurrentSystem(Axiom)
{
}

void LSystem::Run(const int count) {
	Reset();
	for (int i = 0; i < count; i++) {
		Iterate();
	}
}

void LSystem::AddRule(char predecessor, string successor) {
}

void LSystem::Iterate() {
	
}

void LSystem::Reset() {
	m_CurrentSystem = m_Axiom;
}
