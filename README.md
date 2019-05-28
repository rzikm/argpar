# argpar - the argument parsing library for C++
Argpar library provides a quick way of defining command line options/arguments for c++ programs.

## Features:
- Declarative definition of program options + arguments
- support for `--long-option` and short (`-s`) options
- specifying multiple flags with one token (e.g. `-abc` instead of `-a` `-b` `-c`)
- *condensing* the option parameter with the option (`-n10` instead of `-n 10`)
  - same applies for long options: `--long=10` is equivalent to `--long 10`
- Extracting of argument values to program variables
- Ability to add constraints to arguments (e.g. range of allowed values)
- Automatic usage generation

## Usage

The library is header-only and all needed sources are contained in the `include` directory. In the
simplest case, only `#include<argpar/parser.h>` is needed. The main class to be used is the
`argpar::parser` class.

### Simple example

TODO, for now, see [full example](#full-example)

### Specifying options

Program options are specified by calling the `option()` method on the parser:

#### Optional options

Following code snippet sets up a verbose flag for the program, which can be enabled by passing
either `-v` or `--verbose`.

```cpp
bool verbose;
parser.option({"v", "verbose"}, &verbose, "Produces verbose output");
```

If the `-v` or `--verbose` flag is present on the parsed command line, then the `verbose` variable
is set to true. Options do not need to have both short and long names, eigher one suffices.

#### Mandatory options

Sometimes you may require an option to be always present. In that case, there having a boolean
variable signaling presence of the option does not make sense. Calling the overload of `option`
without the `bool *` argument makes the option mandatory.

```cpp
parser.option({"x"}, "A mandatory flag for this program.");
```

Such flags are not very useful, unless the option takes a parameter:

#### Options with parameters

Options can be configured to require a parameter. Following code defines (mandatory) option taking a
string argument:

```cpp
std::string output_file;
parser.option({"o", "outfile"}, "The output file name").string_val("FILE", &output_file);
```

The above specification would cause the `output_file` to be set to `out` when encountering any of
the following on the command line.

```
-oout
-o out
--outfile out
--outfile=out
```

The library supports `std::string`, `int` and `double` option parameters, and can be extended to be
able to accept any user defined parameter, see [this section](#defining-custom-value-types).

#### Constraining the parameter values

The argpar library can do some parameter checking. Following code sets up (this time optional)
option accepting a number between 1 and 10 (both inclusive).

```cpp
int number;
parser.option({"n"}, "A magic number").int_val("NUMBER", &number).between(1, 10);
```

Following constraints are supported:
- int, double:
  - Min, max bounds
- string:
  - list of allowed values

#### Making the option parameter optional

The argpar library supports optional option parameters by specifying a *default value* for the
parameter. Following code snippet builds on the previous example and makes the magic number default
to 5:

```cpp
int number;
parser.option({"n"}, "A magic number").int_val("NUMBER", &number).between(1, 10).with_default(5);
```

Note that use of optional option parameters is strongly discouraged because such options have
confusing semantics and can lead to ambiguous situations for the parser. Consider option `-o` taking
an optional string parameter and command line `cmd -o -f`. Is `-f` supposed to be the parameter for
`-o`, or standalone flag? The argpar library solves this problem by recognizing **only condensed**
syntax for the options with optional arguments. Therefore, supposing the parameter for `-o`,
`--optional` is optionial, the following two command line

```
cmd -o out
cmd --optional out
```

passes option `-o` and a **positional argument** `out` to the `cmd` program, while

```
cmd -oout
cmd --optional=out
```

passes option `-o` with value `out`.

### Specifying positional arguments

In addition to program options, argpar library allows parsing the positional arguments in a similar
way as option parameters. An argument can be defined by calling the `argument()` method on the parser.

#### Mandatory positional arguments

Following piece of code defines a mandatory positional argument.

```cpp
std::string input_file;
parser.argument().string_val("input" &input_file);
```

#### Optional positional arguments

Similarly to option parameters, positional arguments can be made optional by using the
`with_default` to set the default value for the argument.

```cpp
std::string input_file;
parser.argument().string_val("input" &input_file).with_default("-");
```

Optional arguments cannot be followed by mandatory arguments, because such would lead to ambiguous
situations.

#### Accepting variable number of positional arguments

Programs might accept a list of values in positional arguments (e.g. list of files to process). Such
situations can be specified by the `argument_list()` method on the parser.

```cpp
std::vector<std::string> input_files;
parser.argument_list().string_val("files" &input_files);
```

Note that there can be only one `argument_list()` call and this call cannot be followed by a call to `argument()`.

### Advanced features

#### Defining custom value types.

The argpar library can be extended to parse custom types. This can be done by calling
`custom_val<TConfig>()` on the result of either `option()`, `argument()` or `argument_list()` call.
The `TConfig` template parameter should satisfy following:

- public parameterless constructor
- define TConfig::value_type to be the target type of the value. The type must be
  movable or copyable.
- public instance method TConfig::value_type TConfig::parse(std::string const &) const, which
  parses the given string parameter and returns an instance of the target type. This method
  should throw argpar::format_error to indicate that incompatible option parameter was used.

If the values is configured after the `option()` or `argument()` only, the type must additionally
define:
- public instance method bool TConfig::has_default() const, which returns whether the
  configured parameter/option has default value configured
- public instance method TConfig::value_type get_default() const, which returns the default if
  the above method returns true.

If the values is configured after the `argument_list()` only, the type must additionally
define:
- define TConfig::container to be the type used to store the values.

The above requirements can be easily fulfilled by (publicly) deriving from
`argpar::cfg_base<TConfig, TValue>`, but it is not mandatory to do so.

The `custom_val` function returns the internally constructed `TConfig`, which can be then used in
method chaining to set the constraints for the value.

Following code can be used to parse dates from the command line into the strongly typed `tm` structure

```cpp
#include <iomanip>
#include <sstream>
#include <ctime>

class date_config : public argpar::cfg_base<date_config, std::tm>
{
public:
    std::tm parse(std::string const & input) const
    {
        std::stringstream ss(input);
        std::tm ret;

		if (ss >> std::get_time(&time, format_.c_str()))
            return ret;

        throw argpar::format_error("Invalid date format");
    }
    
    date_config & format(char const * fmt)
    {
        format_ = fmt;
    }
private:
    std::string format_ = "%Y-%m-%dT%H:%M"; // sensible default format
};

// later in code
std::tm date;
parser.argument().custom_val<date_config>("date", &date).format("%Y/%m/%d");
```

### Full Example
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

    // do the actual processing...
}
```

#### Output 1:
Command:

```
example.exe echo 1
```

Output:

```
Usage: example.exe [OPTIONS...] -f <FORMAT> <command> [arguments...]
Mandatory option missing: 'f'.
```
#### Output 2:
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

# TODOS:
- Only std::vector is supported as a container?
- Refactor error handling
