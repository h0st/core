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
 * \file MetaCache.h
 * \brief Header of MetaCache
 * \todo add comment here
 *
 * File description
 *
 * PROJ: OSLL/geo2tag
 * ---------------------------------------------------------------- */


#ifndef _MetaCache_H_E337695F_C96B_4956_A169_5053A7AF8B1A_INCLUDED_
#define _MetaCache_H_E337695F_C96B_4956_A169_5053A7AF8B1A_INCLUDED_

#include <QReadWriteLock>

#include "Channel.h"
#include "User.h"
#include "Session.h"
#include "DataMarks.h"

namespace Core
{
    using namespace common;
 /*!
   * Cache for meta information: Users, Sessions, Channels...
   *
   */
  class MetaCache
  {

      static QList<Channel>     s_channels;
      static QList<Session>     s_sessions;
      static QList<BasicUser>   s_users;

      static QReadWriteLock      s_cacheLock;
      static QReadWriteLock      s_usersLock;
      static QReadWriteLock      s_channelsLock;
      static QReadWriteLock      s_SessionsLock;

  public:

      static void init();
      static BasicUser getUserById(const QString userId);
      static QList<BasicUser> getUsers();
      static QList<Channel> getChannels();

      static bool addChannel(const Channel &channel, const BasicUser &user);
      static bool addUser(const BasicUser &user);
      static bool deleteUser(const BasicUser& user);
      static void insertSession(const Session &session);
      static Channel getChannel(const QString name);
      static QList<Channel> getChannels(const common::BasicUser& user);
      static QList<Channel> getChannelsByOwner(const BasicUser &user);

      static void updateSession(Session &session);

      static void removeSession(const Session& session);

      static Session findSession(const common::BasicUser& user);
      static Session findSession(const QString& token);
      static Channel findChannel(const QString& name);
      static QList<Channel> getSubscribedChannels(const BasicUser& user);

      static bool subscribeChannel(const common::BasicUser& user, const Channel& channel);
      static bool unsubscribeChannel(const common::BasicUser& user, const Channel& channel);

      // Reloading sessions from db (used after adding session)
      static void reloadSessions();

      static bool checkEmail(const QString& email);

      // returns true if user exists and loaded
      static bool checkUser(common::BasicUser &user);

      // returns true if channed exists and user is subscribed
      static bool testChannel(common::BasicUser &user, const Channel& channel);

      static bool writeTag(const Tag& tag);
      static QList<Tag> loadTagsFromChannel(const Channel& channel);

      static BasicUser findUserByName(const QString& name);

  protected:

      static void initUsers();
      static void initSessions();
      static void initChannels();
    
  private:    
    MetaCache(const MetaCache& obj);
    MetaCache& operator=(const MetaCache& obj);

  }; // class MetaCache
  
} // namespace Core

#endif //_MetaCache_H_E337695F_C96B_4956_A169_5053A7AF8B1A_INCLUDED_
