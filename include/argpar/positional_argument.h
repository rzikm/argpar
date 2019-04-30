#ifndef ARGPAR_POSITIONAL_ARGUMENT_H
#define ARGPAR_POSITIONAL_ARGUMENT_H

#include "value_config.h"

namespace argpar::detail
{

class positional_argument : public value_config
{
public:
	positional_argument()
		: found_(false)
	{
	}

	value_handler * handler() const
	{
		return handler_.get();
	}

	bool mandatory() const
	{
		return !handler_->has_default();
	}

	void set_found(bool value)
	{
		found_ = value;
	}

	bool found() const
	{
		return found_;
	}
private:
	bool found_;
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
