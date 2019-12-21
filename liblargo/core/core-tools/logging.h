#ifndef LOGGING_H
#define LOGGING_H

#ifndef __logging_h__

#include <iostream>

#define LOG(a)	std::cout << "[" << #a << "] "
#define LOG_END << std::endl;

#define LOG_T LOG(trace)
#define LOG_D LOG(debug)
#define LOG_I LOG(info)
#define LOG_W LOG(warning)
#define LOG_E LOG(error)
#define LOG_F LOG(fatal)

#endif

#endif
