#ifndef ARGPAR_HELPERS_H
#define ARGPAR_HELPERS_H

#include <string>
#include <sstream>

namespace argpar::helpers
{

template <typename THead, typename ... TTail>
void print_to(std::ostream & stream, THead && head, TTail && ... values)
{
	stream << head;
	if constexpr(sizeof...(values) > 0)
	{
		print_to(stream, values...);
	}
}

template <typename ... T>
std::string make_str(T && ... values)
{
	std::stringstream ss;
	print_to(ss, values...);
	return ss.str();
}

class no_copy_move
{
protected:
	no_copy_move()
	{ // allow default constructor
	}
private:
	no_copy_move(const no_copy_move &) = delete;
	no_copy_move(no_copy_move &&) = delete;
	no_copy_move & operator=(const no_copy_move &) = delete;
	no_copy_move & operator=(no_copy_move &&) = delete;
};

}
 #endif // ARGPAR_HELPERS_H
