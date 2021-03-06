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
#ifndef MANAGER_H_INCLUDED
#define MANAGER_H_INCLUDED

namespace manager
{
    enum managestate
    {
        mgr_timeouts,
        mgr_fsref,
        mgr_stat,
        mgr_sessions,
        mgr_wait
    };

    extern managestate mstate;
    extern bool mgrsess;

    void quit(int sig);
    void sig(int sig);
    void timeouts();
    void wait();
    void *manager(void *unused);
    void fsrefresh();
}

#endif // MANAGER_H_INCLUDED
