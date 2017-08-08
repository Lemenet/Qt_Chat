#pragma once

#include <iostream>

class ChatMessage
{
public:
	enum
	{
		HEADER_LENGTH = 4,    //°üÍ·
							   //Ãû×Ö
		MAX_BODY_LENGTH = 512,//
	};

//#define HEADER_LENGTH 4
//#define MAX_BODY_LENGTH 512

private:
	std::size_t bodyLength_;
	char data_[HEADER_LENGTH + MAX_BODY_LENGTH];

public:
	ChatMessage() :
		bodyLength_(0)
	{

	}

	const char* data() const
	{
		return data_;
	}

	char* data()
	{
		return data_;
	}

	const char* body() const
	{
		return data_ + HEADER_LENGTH;
	}

	char* body()
	{
		return data_ + HEADER_LENGTH;
	}

	std::size_t length()
	{
		return HEADER_LENGTH + bodyLength_;
	}

	std::size_t headLength()
	{
		return HEADER_LENGTH;
	}

	std::size_t bodyLength()
	{
		return bodyLength_;
	}

	void SetBodyLength(size_t new_length)
	{
		bodyLength_ = new_length;
		if (bodyLength_ > MAX_BODY_LENGTH)
		{
			bodyLength_ = MAX_BODY_LENGTH;
		}
	}

	bool DecodeHeader()
	{
		char header[HEADER_LENGTH + 1] = "";
		strncat(header, data_, HEADER_LENGTH);
		bodyLength_ = atoi(header);
		if (bodyLength_ > MAX_BODY_LENGTH)
		{
			bodyLength_ = 0;
			return false;
		}
		return true;
	}
	
		bool EncodeHeader()
	{
		char header[HEADER_LENGTH + 1] = "";
		sprintf(header, "%4d", bodyLength_);
		memcpy(data_, header, HEADER_LENGTH);
		return true;
	}

};
