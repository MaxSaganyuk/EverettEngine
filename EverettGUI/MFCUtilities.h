#include "afxdialogex.h"
#include <vector>

class MFCUtilities
{
public:
	static bool EditIsEmpty(const CEdit& edit)
	{
		return edit.SendMessage(WM_GETTEXTLENGTH) == 0;
	}

	static bool EditsAllEmpty(const std::vector<CEdit*>& edits)
	{
		bool res = true;

		for (const CEdit* edit : edits)
		{
			res &= EditIsEmpty(*edit);

			if (!res)
			{
				return false;
			}
		}

		return true;
	}

	static bool EditsAnyEmpty(const std::vector<CEdit*>& edits)
	{
		bool res = false;

		for (const CEdit* edit : edits)
		{
			res |= EditIsEmpty(*edit);

			if (res)
			{
				return true;
			}
		}

		return res;
	}
};