#include "parser_fixture.h" 

void parser_fixture::parse(std::vector<std::string> tokens)
{
	std::vector<char *> args(tokens.size() + 2);
	char cmd[] = "cmd";
	args[0] = cmd;
	args[tokens.size() + 1] = nullptr;
	for (size_t i = 0; i < tokens.size(); ++i)
	{
		args[i + 1] = tokens[i].data();
	}

	parser.parse(static_cast<int>(args.size()) - 1, args.data());
}
