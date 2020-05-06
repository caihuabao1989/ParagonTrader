#include "Message.h"

bool operator<(const Sender& lhs, const Sender& rhs)
{
	return lhs.q < rhs.q;
}
