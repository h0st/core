/*
 * Copyright 2013  Kirill Krinkin  kirill.krinkin@gmail.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * The advertising clause requiring mention in adverts must never be included.
 */

/*! ---------------------------------------------------------------
 *
 * \file MetaCache.cpp
 * \brief MetaCache implementation
 *
 * File description
 *
 * PROJ: OSLL/geo2tag
 * ---------------------------------------------------------------- */

#include "MetaCache.h"
#include "QueryExecutor.h"
#include "servicelogger.h"
#include "defines.h"

namespace Core
{

QList<Channel>           MetaCache::s_channels;
QList<Session>           MetaCache::s_sessions;
QList<BasicUser>         MetaCache::s_users;

QReadWriteLock      MetaCache::s_cacheLock;
QReadWriteLock      MetaCache::s_usersLock;
QReadWriteLock      MetaCache::s_channelsLock;
QReadWriteLock      MetaCache::s_SessionsLock;

void MetaCache::init()
{
    DEBUG() << "Initializing MetaCache objects";
    initUsers();
    initChannels();
}

BasicUser MetaCache::getUserById(const QString userId)
{
    QReadLocker lock(&s_usersLock);

    common::BasicUser realUser;      // Null pointer

    BasicUser user;
    foreach(user,s_users)
    {
        if(QString::compare(user.getEmail(), userId, Qt::CaseInsensitive) == 0)
            return user;
    }

    return BasicUser();
}

QList<BasicUser> MetaCache::getUsers()
{
    QReadLocker lock(&s_usersLock);
    return   s_users;
}

QList<Channel> MetaCache::getChannels()
{
    QReadLocker lock(&s_channelsLock);
    return   s_channels;
}

bool MetaCache::addChannel(const Channel &channel, const common::BasicUser& user)
{
    bool result = QueryExecutor::instance()->insertNewChannel(channel, user);
    
    if (result){
      DEBUG() << "Adding to memory channel " << channel.getName();
      s_channels.push_back(Channel(channel));
    }
    return result;
}

bool MetaCache::addUser(const common::BasicUser &user)
{
    bool result = QueryExecutor::instance()->insertNewUser(user);
    if (result)
    {
        QWriteLocker lock(&s_usersLock);
	s_users.push_back(user);
    }
    return result;
}

bool MetaCache::deleteUser(const BasicUser &user)
{
    bool result = QueryExecutor::instance()->deleteUser(user);

    if (result)
    {
      BasicUser u;
      foreach (u, s_users)
      {
        if (u.getLogin() == user.getLogin())
	{
          QWriteLocker lock(&s_usersLock);
          s_users.removeOne(u);
	  break;
        }

      }
    }   

    return result;
}

void MetaCache::insertSession(const Session& session)
{
    s_sessions << session;
    DEBUG() << "inserted " << session << ", session count=" << s_sessions.size();
}

Channel MetaCache::getChannel(const QString name)
{
    return QueryExecutor::instance()->getChannel(name);
}

QList<Channel> MetaCache::getChannelsByOwner(const BasicUser &user)
{
    return QueryExecutor::instance()->getChannelsByOwner(user);
}

Session MetaCache::findSession(const BasicUser &user)
{
    QReadLocker lock(&s_SessionsLock);

    Session s;
    foreach(s,s_sessions)
    {
        if(s.getUser() == user )
        {
	    if (!s.isExpired())
	    {
              return s;
            }
	    else{
	      removeSession(s);
              continue;
            }
        }

    }
    return Session();
}

void MetaCache::removeSession(const Session& session){
  s_sessions.removeOne(session);
}


Session MetaCache::findSession(const QString &token)
{
    DEBUG() << "Looking op for a session, token " << token;
    Session s;
    foreach(s,s_sessions)
    {
        DEBUG() << "test:" << s;
        if(s.getSessionToken() == token )
        {
	    if (!s.isExpired())
	    {
              DEBUG() << "...found " << s;
              return s;
            }
	    else{
	      removeSession(s);
              break;
            }

        }
    }
    DEBUG() << "...not found ";
    return Session();
}

Channel MetaCache::findChannel(const QString &name)
{
#ifdef GEO2TAG_LITE
    return QueryExecutor::instance()->getChannel(name);
#else
    Channel ch;
    QReadLocker lock(&s_channelsLock);
    foreach(ch,s_channels)
    {
	DEBUG() << "Checking channel " << ch.getName(); 
        if(QString::compare(ch.getName(), name, Qt::CaseInsensitive) == 0)
            return ch;
    }
    return Channel();
#endif
}

QList<Channel> MetaCache::getSubscribedChannels(const BasicUser &user)
{
    return QueryExecutor::instance()->getSubscribedChannels(user);
}

bool MetaCache::subscribeChannel(const BasicUser &user, const Channel &channel)
{
    return QueryExecutor::instance()->subscribeChannel(user,channel);
}

bool MetaCache::unsubscribeChannel(const BasicUser &user, const Channel &channel)
{
    return QueryExecutor::instance()->unsubscribeChannel(user,channel);
}

void MetaCache::reloadSessions()
{
    QWriteLocker lock(&s_SessionsLock);
    initSessions();
}

bool MetaCache::checkEmail(const QString& email){

  bool result = QueryExecutor::instance()->checkEmail(email);
  return result;

}

bool MetaCache::checkUser(BasicUser &user)
{

    BasicUser bu = QueryExecutor::instance()->getUser(user.getLogin());


    DEBUG() << "Checking " << user << " ->" << bu.isValid();

    return bu.isValid();
}

bool MetaCache::testChannel(BasicUser &user, const Channel& channel)
{
    DEBUG() << "check channel " << channel.getName() << " for " << user.getLogin();

    if(!channel.isValid())
    {
        WARNING() << "Channel " << channel << " is invalid";
        return false;
    }

#ifdef GEO2TAG_LITE
    return true;
#else
    return QueryExecutor::instance()->isSubscribed(user,channel);
#endif
}

bool MetaCache::writeTag(const Tag &tag)
{
    return QueryExecutor::instance()->insertNewTag(tag);
}

QList<Tag> MetaCache::loadTagsFromChannel(const Channel &channel)
{
    return QueryExecutor::instance()->loadTags(channel);
}

void MetaCache::initUsers()
{
    QWriteLocker lock(&s_cacheLock);

    DEBUG() << "Initializing Users";
    s_users=QueryExecutor::instance()->loadUsers();
    DEBUG() << "Loaded " << s_users.size() << "users";
    BasicUser u;
    foreach(u, s_users)
    {
       DEBUG() << u;
    }
}

BasicUser MetaCache::findUserByName(const QString& name){
    BasicUser u;
 
    foreach(u,s_users)
    {
        if(QString::compare(u.getLogin(), name, Qt::CaseInsensitive) == 0 )
            return u;
    }

    return BasicUser();

}

void MetaCache::updateSession(Session &session)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    session.setLastAccessTime(currentTime);
}

void MetaCache::initSessions()
{
#if GEO2TAG_LITE
    DEBUG() << "Session persistans is not supported in lite version";
#else
    NOT_IMPLEMENTED();
#endif
}

void MetaCache::initChannels()
{
    QWriteLocker lockf(&s_channelsLock);
#ifdef GEO2TAG_LITE
    // initialization is not supported;
#else
    QWriteLocker lock(&s_cacheLock);

    DEBUG() << "Initializing Channels";
    s_channels=QueryExecutor::instance()->loadChannels();
    DEBUG() << "Loaded " << s_users.size() << "channels";
#endif
}

} // namespace Core
