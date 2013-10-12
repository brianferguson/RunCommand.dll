/*
  Copyright (C) 2013 Brian Ferguson

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "StdAfx.h"

#ifndef __PLUGINRUNCOMMAND_H__
#define __PLUGINRUNCOMMAND_H__

struct Measure
{
	// Options
	std::wstring program;
	std::wstring parameter;
	std::wstring finishAction;
	std::wstring outputFile;
	std::wstring folder;
	WORD state;
	int timeout;

	// Internal values
	std::wstring result;
	std::recursive_mutex mutex;
	bool threadActive;
	void* skin;

	Measure() :
		program(),
		parameter(),
		finishAction(),
		outputFile(),
		folder(),
		state(0),
		timeout(-1),
		result(),
		mutex(),
		threadActive(false),
		skin()
		{ }
};

#endif

std::wstring Widen(const char* str, int strLen = -1, int cp = CP_ACP)
{
	std::wstring wideStr;

	if (str && *str)
	{
		if (strLen == -1)
		{
		strLen = (int)strlen(str);
		}

		int bufLen = MultiByteToWideChar(cp, 0, str, strLen, nullptr, 0);
		if (bufLen > 0)
		{
			wideStr.resize(bufLen);
			MultiByteToWideChar(cp, 0, str, strLen, &wideStr[0], bufLen);
		}
	}
	return wideStr;
}
