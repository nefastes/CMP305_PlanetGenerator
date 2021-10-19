#include "LSystem.h"

LSystem::LSystem(std::string Axiom) :
	m_Axiom(Axiom), m_CurrentSystem(Axiom)
{
}

void LSystem::Run(const unsigned count) {
	Reset();
	for (unsigned i = 0u; i < count; i++) Iterate();
}

void LSystem::AddRule(char predecessor, const std::string& successor) {
	m_rules.emplace(predecessor, successor);
}

void LSystem::Iterate() {
	std::string newSystem("");
	for (auto it : m_CurrentSystem)
	{
		std::string successor = m_rules.at(it);	//m_CurrentSystem[i]
		newSystem.append(successor);
	}
	m_CurrentSystem.assign(newSystem);
}

void LSystem::Reset() {
	m_CurrentSystem.assign(m_Axiom);
}
