#ifndef PARSER_FIXTURE_H
#define PARSER_FIXTURE_H

#include <gtest/gtest.h>
#include <argpar/argpar.h>

class parser_fixture : public ::testing::Test
{
protected:
	parser_fixture()
		: o_val(0)
		, m_val(0)
	{
	}

	void parse(std::vector<std::string> tokens);

	int o_val;
	int m_val;
	argpar::parser parser;
};

#endif // PARSER_FIXTURE_H
 
