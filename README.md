# argpar - the argument parsing library for C++
Argpar library provides a quick way of defining command line options/arguments for c++ programs.

## Features:
- Declarative definition of program options + arguments
- Extracting of argument values to program variables
- Automatic usage generation

## Example:
Following code shows how the library can be used to define arguments for a simplified `time` command.

```cpp
#include <argpar/argpar.h>

#include <iostream>
#include <vector>

int main(int argc, char * argv[])
{
	bool print_version;
	std::string format;
	std::string command;
	std::vector<std::string> commandArgs;
	bool verbose;
	bool print_help;

	argpar::parser parser;
	parser.option({"V", "version"}, &print_version,
		"Prints out version and exits successfully"); 
	parser.option({"f", "format"},
		"Sets format for the output.") 
	      .string_val("FORMAT", &format); 
	parser.option({"v", "verbose"}, &verbose,
		"Enables verbose output.");
	parser.option({"help"}, &print_help,
		"Prints out usage and exits successfully");

	parser.argument().string_val("command", &command);
	parser.argument_list().string_val("arguments", &commandArgs);

	try
	{
		parser.parse(argc, argv);
	}
	catch (argpar::parse_error & e)
	{
		if (!print_help)
		{
			parser.print_usage(std::cout);
			std::cout << e.what() << std::endl;
			return -1;
		}
	}
    
	if (print_help)
	{
		parser.print_help(std::cout);
		return 0;
	}

    //...
}
```

### Output 1:
Command:

```
example.exe echo 1
```

Output:

```
Usage: example.exe [OPTIONS...] -f <FORMAT> <command> [arguments...]
Mandatory option missing: 'f'.
```
### Output 2:
Command:

```
example.exe --help
```

Output:
```
Usage: example.exe [OPTIONS...] -f <FORMAT> <command> [arguments...]
Options:
  -V, --version
        Prints out version and exits successfully

  -f, --format <FORMAT>                                              (mandatory)
        Sets format for the output.

  -v, --verbose
        Enables verbose output.

      --help
        Prints out usage and exits successfully

```
