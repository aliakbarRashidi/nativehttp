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


#include "../nativehttp.h"
#include "../protocol.h"
#include "executor.h"
#include "data.h"
#include "in.h"
#include "sender.h"

namespace http
{

int executor(void* eid)
{
    http::Sexecutor* exc=(http::Sexecutor*)eid;
    exc->state=-1;
    rdata rd;
    http::rproc::lrqd ld;
    int ts=0;
    while(true)
    {
        SDL_mutexP(http::mtx_exec);
        if(http::toexec.size()<=0)
        {
            SDL_mutexV(http::mtx_exec);
            SDL_Delay(1);
            continue;
        }
        if(http::toexec.front(ts)->taken>0)
        {
            SDL_mutexV(http::mtx_exec);
            SDL_Delay(1);
            continue;
        }
        if(ts==1)
        {
            SDL_mutexV(http::mtx_exec);
            SDL_Delay(1);
            continue;
        }
        http::toexec.front()->taken=exc->id;
        http::request* process=http::toexec.front(ts);
        if(ts==1)
        {
            SDL_mutexV(http::mtx_exec);
            SDL_Delay(1);
            continue;
        }
        if(http::toexec.front()->taken!=exc->id)//it's just impossible, but...
        {
            log("IMPOSSIBLE ERROR","This error is impossible to occur, if you see this, god will left you..");
            SDL_mutexV(http::mtx_exec);
            SDL_Delay(1);
            continue;
        }
        http::toexec.pop();
        SDL_mutexV(http::mtx_exec);


        rd.get=NULL;
        rd.post=NULL;
        rd.cookie=NULL;

        ld.clen=0;

        http::rproc::line0(process,rd,ld);

        if(process->method==0)
        {

            http::sender::send(process->uid,http::error::e400.size,http::error::e400.data,false);
            http::unlockclient(process->uid);
            delete[] process->request;
            continue;
        }
        if(process->method==3)
        {
            http::sender::send(process->uid,http::error::e501.size,http::error::e501.data,false);
            http::unlockclient(process->uid);
            delete[] process->request;
            continue;
        }
        if(!process->http11)
        {
            http::sender::send(process->uid,http::error::e505.size,http::error::e505.data,false);
            http::unlockclient(process->uid);
            delete[] process->request;
            delete process;
            continue;
        }

        http::rproc::header(process,rd,ld);


        if(!rd.cookie)
        {
            rd.cookie=new cookiedata("");
        }

        if(ld.clen>0&&process->method==2)
        {
            if(ld.clen>http::maxPost)
            {
                if(rd.cookie)
                {
                    delete rd.cookie;
                }
                if(rd.get)
                {
                    delete rd.get;
                }
                if(rd.post)
                {
                    delete rd.post;
                }
                exc->fd1=NULL;
                exc->fd2=NULL;
                exc->state=time(0);
                exc->in=3;
                http::sender::sendNow(process->uid,http::error::e403.size,http::error::e403.data,false);
                exc->state=-1;
                exc->in=0;
                http::kickclient(process->uid);
                delete[] process->request;
                delete process;
                continue;
            }
            exc->fd1=rd.cookie;
            exc->fd2=rd.get;
            exc->state=time(0);
            exc->in=1;
            http::rproc::post(rd,process,ld);
            exc->state=-1;
            exc->in=0;
            if(!rd.post)
            {
                delete[] process->request;
                delete process;
                process->request=NULL;
                if(rd.cookie)
                {
                    delete rd.cookie;
                }
                if(rd.get)
                {
                    delete rd.get;
                }
                if(rd.post)
                {
                    delete rd.post;
                }
                http::unlockclient(process->uid);
                continue;//will be disconnected
            }
        }
        delete[] process->request;
        process->request=NULL;

        rd.remoteIP=SDLNet_TCP_GetPeerAddress(http::connected[process->uid])->host;

        pagedata result;
        exc->fd1=rd.cookie;
        exc->fd2=rd.get;
        exc->in=2;
        exc->state=time(0);
        if(http::rproc::ex(result,&rd))
        {
            exc->state=-1;
            exc->in=0;
            if(rd.cookie)
            {
                delete rd.cookie;
            }
            if(rd.get)
            {
                delete rd.get;
            }
            if(rd.post)
            {
                delete rd.post;
            }
            http::sender::send(process->uid,http::error::e404.size,http::error::e404.data,false);
            http::unlockclient(process->uid);
            delete process;
            process=NULL;
            continue;
        }
        exc->state=-1;
        exc->in=0;

        if(rd.cookie)
        {
            delete rd.cookie;
        }
        if(rd.get)
        {
            delete rd.get;
        }
        if(rd.post)
        {
            delete rd.post;
        }

        if(result.data)
        {
            SDL_mutexP(http::mtx_snd);
            http::sender::send(process->uid,result.size,result.data,true);
            SDL_mutexV(http::mtx_snd);
        }

        http::unlockclient(process->uid);
        delete process;
        process=NULL;
    }
    return 1;
}

namespace rproc
{

void header(http::request* process,rdata& rd, http::rproc::lrqd& ld)
{
    superstring hss(process->request);
    hss.to("\r\n");
    hss.str=hss.to("\r\n\r\n");
    hss.pos=0;

    hss.add_token(token("\r\n\r\n",0));
    hss.add_token(token("Host: ",1));
    hss.add_token(token("User-Agent: ",2));
    hss.add_token(token("Referer: ",3));
    hss.add_token(token("Cookie: ",4));
    hss.add_token(token("Content-Length: ",5));

    while(hss.pos<hss.str.size())
    {
        token pt=hss.tok();
        if(pt.id==0)break;
        switch(pt.id)
        {
        case 1:
            rd.host=hss.to("\r\n");
            break;
        case 2:
            rd.userAgent=hss.to("\r\n");
            break;
        case 3:
            rd.referer=hss.to("\r\n");
            break;
        case 4:
            rd.cookie=new cookiedata(hss.to("\r\n"));
            break;
        case 5:
            ld.clen=strtol(hss.to("\r\n").c_str(), NULL, 10);
            break;
        }
    }
    hss.str.clear();

}

void post(rdata& rd, http::request* process, http::rproc::lrqd& ld)
{
    superstring ars(process->request);
    ars.str=ars.from("\r\n\r\n");
    if(ars.str.size()<ld.clen)
    {
        unsigned int ltrv=ld.clen-ars.str.size();
        char* tv=new char[ltrv+1];
        unsigned int ar=0;
        while(0<ltrv)
        {
            int rv=SDLNet_TCP_Recv(process->sender,tv+ar,ltrv);
            if(rv==-1)
            {
                delete[] tv;
                tv=NULL;
                break;
            }
            ar+=rv;
            ltrv-=rv;
            (tv+ar)[0]=0;
        }
        if(tv)
        {
            ars.str+=tv;
            delete[] tv;
        }
    }
    rd.post=new postgetdata(ars.str);
}

bool ex(pagedata& pd,rdata* rd)
{
    page pid = pmap.by_uri(rd->uri.c_str());

    switch(pid.type)
    {
    case page_native:
    {
        rd->ctype="text/html;charset="+charset;
        rd->response="200 OK";

        nativepage *npp = (nativepage*)pid.data;
        SDL_mutexP(http::mtx_exec2);
        pagedata ts=npp->page(rd);//<<<execution of page
        SDL_mutexV(http::mtx_exec2);
        string snd = "HTTP/1.1 "+rd->response+"\r\n"+http::headers::standard;
        snd+=http::headers::alive+http::headers::alivetimeout;
        snd+="Content-Type: "+rd->ctype+"\r\nContent-Length: ";
        snd+=its(ts.size);
        snd+="\r\n";
        snd+=rd->cookie->gethead();
        snd+="\r\n";
        string snd2="\r\n";

        pd.size=snd.size()+ts.size+snd2.size();
        pd.data=new char[pd.size];
        memcpy(pd.data,snd.c_str(),snd.size());
        memcpy(pd.data+snd.size(),ts.data,ts.size);
        memcpy(pd.data+snd.size()+ts.size,snd2.c_str(),snd2.size());
        delete[]ts.data;
        return false;
    }
    break;
    case page_file:
    {

        FILE* f=fopen((const char*)pid.data,"r");
        if(f)
        {
            fseek(f,0,SEEK_END);
            int size = ftell(f);
            fseek(f,0,SEEK_SET);
            rewind(f);
            string snd("HTTP/1.1 200 OK\r\n");
            snd += http::headers::standard;
            snd += http::headers::alive+http::headers::alivetimeout;
            snd += mime->get_ctype((char*)pid.data);
            snd += "\r\nContent-Length: ";
            snd += its(size);
            snd += "\r\n\r\n";
            char* b=new char[size];
            fread(b,1,size,f);
            string snd2="\r\n";
            pd.size=size+snd.size()+snd2.size();
            pd.data=new char[pd.size];
            memcpy(pd.data,snd.c_str(),snd.size());
            memcpy(pd.data+snd.size(),b,size);
            memcpy(pd.data+snd.size()+size,snd2.c_str(),snd2.size());
            delete[] b;
            fclose(f);
            return false;
        }
        else
        {
            return true;
        }
    }
    break;
    default:
        return true;
    }
    return true;

}

void line0(http::request* process,rdata& rd, http::rproc::lrqd& ld)
{
    superstring rss(superstring(process->request).to("\r\n"));
    rss.add_token(token(" ",0));
    rss.add_token(token("GET",1));
    rss.add_token(token("HEAD",3));
    rss.add_token(token("POST",2));
    rss.add_token(token("DELETE",3));
    rss.add_token(token("TRACE",3));
    rss.add_token(token("CONNECT",3));
    rss.add_token(token("OPTIONS",3));

    process->method=rss.tok().id;

    rss.clear_tokens();
    rss.pos++;
    superstring rawuri(rss.to(" "));
    process->http11=rss.check("HTTP/1.1");

    rd.uri=rawuri.to("?");
    string gu=rawuri.to("#");
    if(!gu.empty())
    {
        rd.get=new postgetdata(gu);
    }
}

}//namespace rproc


}//http namespace
