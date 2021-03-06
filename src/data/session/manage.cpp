/*
Copyright (c) 2013 Lukasz Magiera

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/
#include "session.h"
#include "http/data.h"

void sdata::session::sstg::mng()
{
	for (size_t i = 0; i < scount; i++)
	{
		if (data[i].started)
		{
			if (data[i].started + http::sess_life < time(0))
			{
				data[i].started = 0;
				data[i].svid = 0;
				data[i].data.free();
				active--;
			}
		}
	}
}

void sdata::session::sbmain::free()
{
	if (!keys)
	{
		fileds = 0;
		return;
	}

	for (size_t i = 0; i < fileds; i++)
	{
		if (keys[i])
		{
			delete keys[i];
		}
	}
	if (keys)
		delete[] keys;
	keys = NULL;
	fileds = 0;
}
