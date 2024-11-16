#pragma once
#include <string>
#include <fstream>
#include <cassert>
#include <iostream>
#include <DirectXMath.h>
#include <type_traits>

class Logger
{
public:
	~Logger()
	{
		CloseFile();
	}

	static Logger& Get()
	{
		static Logger instance;
		return instance;
	}

	void OpenFile(const std::string& filename)
	{
		file.open(filename);
		assert(isFileOpen() && "Failed to open file");
	}

	void CloseFile()
	{
		if (isFileOpen())
			file.close();
	}

	template<typename... Args>
	void LogHeader(Args&&... args)
	{
		// Just redirect to log
		Log(0.f, std::forward<Args>(args)...);	
	}

	// Generic log function redirects to private function
	template<typename... Args>
	void Log(Args&&... args)
	{
		assert(isFileOpen() && "file isn't open");

		LogValues(std::forward<Args>(args)...);
		file << std::endl;
	}

private:
	// Recursive variadic template
	template<typename T, typename... Args>
	void LogValues(T&& first, Args&&... rest)
	{
		LogValue(std::forward<T>(first));
		if constexpr (sizeof...(rest) > 0)
		{
			file << ", ";
			LogValues(std::forward<Args>(rest)...);
		}
	}
	// Base case for no args
	void LogValues() {}

	// Overload for general types using SFINAE
	template<typename T>
	typename std::enable_if<!std::is_same<T, DirectX::XMFLOAT3>::value && !std::is_same<T, DirectX::XMVECTOR>::value>::type
	LogValue(T&& value)
	{
		file << value;
	}

	// Overload for DirectX::XMFLOAT3
	void LogValue(const DirectX::XMFLOAT3& vec)
	{
		file << vec.x << ", " << vec.y << ", " << vec.z;
	}

	// Overload for DirectX::XMVECTOR
	void LogValue(const DirectX::XMVECTOR& vec)
	{
		DirectX::XMFLOAT4 float4;
		DirectX::XMStoreFloat4(&float4, vec);
		file << float4.x << ", " << float4.y << ", " << float4.z << ", " << float4.w;
	}

private:
	Logger() = default;
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	bool isFileOpen() const
	{
		return file.is_open();
	}

private:
	std::ofstream file;
};

