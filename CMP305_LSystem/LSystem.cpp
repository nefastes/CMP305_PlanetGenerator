#include "LSystem.h"

LSystem::LSystem(std::string& Axiom) :
	m_Axiom(Axiom), m_CurrentSystem(Axiom)
{
}

void LSystem::Run(const unsigned count) {
	Reset();
	for (unsigned i = 0u; i < count; i++) Iterate();
}

void LSystem::ClearRules()
{
	m_rules.clear();
}

void LSystem::AddRule(char predecessor, const std::string& successor) {
	m_rules.emplace(predecessor, successor);
}

void LSystem::Iterate() {
	std::string newSystem("");
	for (auto key : m_CurrentSystem)
	{
		std::string successor;
		//Retrieve the number of elements for this key
		size_t n_elements = m_rules.count(key);
		assert(n_elements != 0);

		std::multimap<char, std::string>::iterator item = m_rules.find(key);
		if (n_elements > 1)
		{
			//If we have more than one element, randomly select one of the multiple elements
			//All ellements should be selected in equal weights
			float weight = 100.f / static_cast<float>(n_elements);
			float selection = static_cast<float>(rand()% 100);
			selection /= weight;
			assert(selection < 8.f);
			for (size_t i = 0u; i < static_cast<size_t>(selection); ++i) ++item;
		}

		//Now assign the successor and add it to the new system
		successor = item->second;
		newSystem.append(successor);
	}
	m_CurrentSystem.assign(newSystem);
}

void LSystem::Reset() {
	m_CurrentSystem.assign(m_Axiom);
}
