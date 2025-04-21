#pragma once

#include <memory>

namespace Tokens
{
	class TokenSource;
	class LifetimeToken
	{
	private:
		bool Valid = true;

		friend TokenSource;
		void Invalidate() { Valid = false; };

		size_t ID = 0;
	public:
		bool IsValid() const { return Valid; }
		LifetimeToken()
		{
			ID = size_t(this);
		}
		
		size_t GetID() const { return ID; }

		// noncopyable
		LifetimeToken(const LifetimeToken&) = delete;
		LifetimeToken& operator = (const LifetimeToken&) = delete;

	};

	using LifetimeTokenPtr = std::shared_ptr<LifetimeToken>;

	class TokenSource
	{
	private:
		LifetimeTokenPtr RootToken;
	public:
		TokenSource()
		{
			RootToken = std::make_shared<LifetimeToken>();
		}

		~TokenSource()
		{
			RootToken->Invalidate();
		}

		// noncopyable
		TokenSource(const TokenSource&) = delete;
		TokenSource& operator = (const TokenSource&) = delete;

		LifetimeTokenPtr GetToken() const
		{
			return RootToken;
		}
	};
}