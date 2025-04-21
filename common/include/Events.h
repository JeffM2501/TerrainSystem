#pragma once

#include "LifetimeToken.h"
#include <functional>

namespace Events
{
	template<class T>
	class EventSource
	{
	private:
		struct EventHandler
		{
			std::function<void(const T&)> Handler;
			Tokens::LifetimeTokenPtr Token;
		};

		std::vector<EventHandler> EventHandlers;
	public:

		void Add(std::function<void(const T&)> handler, Tokens::LifetimeTokenPtr token)
		{
			// ensure that we only do unique items, one per token
			for (auto& record : EventHandlers)
			{
				if (record.Token == token)
				{
					record.Handler = handler;
					return;
				}
			}

			EventHandlers.emplace_back(EventHandler{ handler, token });
		}

		void Remove(Tokens::LifetimeTokenPtr token)
		{
			for (auto itr = EventHandlers.begin(); itr != EventHandlers.end(); itr++)
			{
				if (itr->Token->GetID() == token->GetID())
				{
					EventHandlers.erase(itr);
					return;
				}
			}
		}

		void Invoke(const T& argument)
		{
			for (auto itr = EventHandlers.begin(); itr != EventHandlers.end();)
			{
				if (itr->Token->IsValid())
				{
					itr->Handler(argument);
					itr++;
				}
				else
				{
					// if the token is not valid, remove from the list of events as the function pointer is not valid anymore
					itr = EventHandlers.erase(itr);
				}
			}
		}
	};
}