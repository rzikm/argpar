#ifndef ARGPAR_POSITIONAL_ARGUMENT_H
#define ARGPAR_POSITIONAL_ARGUMENT_H

#include "value_config.h"

namespace argpar::detail
{

class positional_argument : public value_config
{
public:
	value_handler * handler() const
	{
		return handler_.get();
	}
};

class positional_argument_list : public value_list_config
{
public:
	value_handler * handler() const
	{
		return handler_.get();
	}
};

}

#endif // ARGPAR_POSITIONAL_ARGUMENT_H
