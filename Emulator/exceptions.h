#pragma once
#include <exception>

#if defined _MSC_VER
#if _MSC_VER >= 1900
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif
#else
#define NOEXCEPT noexcept
#endif

class GameBoyException : public std::exception{
public:
	virtual ~GameBoyException(){}
	virtual GameBoyException *clone() = 0;
};

class GenericException : public GameBoyException{
	const char *cmessage;
public:
	GenericException(const char *message): cmessage(message){}
	virtual ~GenericException(){}
	virtual GameBoyException *clone() override{
		return new GenericException(this->cmessage);
	}
	virtual const char *what() const NOEXCEPT{
		return this->cmessage;
	}
};

class NotImplementedException : public GenericException{
public:
	NotImplementedException(): GenericException("Not implemented."){}
	GameBoyException *clone() override{
		return new NotImplementedException();
	}
};
