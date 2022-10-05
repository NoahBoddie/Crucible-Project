#pragma once

namespace Crucible
{

	template<class... AcceptArgs>
	class CompetitiveEvent
	{
	public:
		void Register()
		{
			_registered = true;
		}

		void Unregister()
		{
			if (!_registered)
				return;

			_registered = false;
		}

		bool TryResult(bool success, AcceptArgs... args)
		{
			if (success)
				return TrySucces(args...);
			
			return TryFailure(args...);
		}


		bool TrySuccess(AcceptArgs... args)
		{
			if (!_registered)
				return false;

			if (OnResult(true, args...) == false)
				Unregister();
			return true;

		}

		bool TryFailure(AcceptArgs... args)
		{
			if (!_registered)
				return false;

			if (OnResult(false, args...) == false)
				Unregister();
			return true;
		}

		

	protected:
		bool _registered;//For now this will do, when 0 unregistered, when 1 registered

		//Returning false will cause it end, returning true will register it to run again.
		virtual bool OnResult(bool success, AcceptArgs... args) = 0;

	};

}