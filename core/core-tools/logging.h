#ifndef LOGGING_H
#define LOGGING_H

#ifndef __logging_h__

#include <iostream>

#define LOG(a)	std::cout << "[" << #a << "] "
#define LOG_END << std::endl;

#endif

#endif
