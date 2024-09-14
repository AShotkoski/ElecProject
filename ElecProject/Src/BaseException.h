#pragma once
#include <exception>
#include <string>

class BaseException : public std::exception
{
public:
	BaseException( int line, const std::string& file ) noexcept;
	virtual ~BaseException() = default;
	virtual const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
private:
	int line;
	std::string file;
protected:
	// Used for what function since pointer
	mutable std::string whatBuffer;
};

