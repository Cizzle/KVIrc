//=============================================================================
//
//   File : KviProxyDataBase.cpp
//   Creation date : Sat Jul 22 2000 18:23:23 by Szymon Stefanek
//
//   This file is part of the KVIrc IRC client distribution
//   Copyright (C) 2000-2010 Szymon Stefanek (pragma at kvirc dot net)
//
//   This program is FREE software. You can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the HOPE that it will be USEFUL,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program. If not, write to the Free Software Foundation,
//   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
//=============================================================================

#include "KviProxyDataBase.h"
#include "KviConfigurationFile.h"
#include "KviCString.h"
#include "KviProxy.h"

#include <QString>

void KviProxyDataBase::updateProxyIp(const char * proxy, const char * ip)
{
	for(auto & prx : m_pProxyList)
	{
		if(QString::compare(proxy, prx->m_szHostname, Qt::CaseInsensitive))
		{
			prx->m_szIp = ip;
			return;
		}
	}
}

KviProxy * KviProxyDataBase::findProxy(const KviProxy * pProxy, bool bName)
{
	for(auto & p : m_pProxyList)
	{
		if(bName)
		{
			if(QString::compare(p->m_szHostname, pProxy->m_szHostname, Qt::CaseInsensitive))
				return p.get();
		}
		else
		{
			if(QString::compare(p->m_szHostname, pProxy->m_szHostname, Qt::CaseInsensitive) && (p->m_uPort == pProxy->m_uPort) && (p->protocol() == pProxy->protocol()) && (p->isIPv6() == pProxy->isIPv6()))
				return p.get();
		}
	}
	return nullptr;
}

void KviProxyDataBase::clear()
{
	m_pProxyList.clear();
	m_pCurrentProxy = nullptr;
}

void KviProxyDataBase::load(const QString & filename)
{
	clear();
	KviConfigurationFile cfg(filename, KviConfigurationFile::Read);

	unsigned int nEntries = cfg.readUIntEntry("Entries", 0);

	for(unsigned int i = 0; i < nEntries; i++)
	{
		std::unique_ptr<KviProxy> p(new KviProxy());
		KviCString tmp(KviCString::Format, "%u_Hostname", i);
		p->m_szHostname = cfg.readEntry(tmp.ptr(), "proxy.example.net");
		tmp.sprintf("%u_Port", i);
		p->m_uPort = cfg.readUIntEntry(tmp.ptr(), 7000);
		tmp.sprintf("%u_Ip", i);
		p->m_szIp = cfg.readEntry(tmp.ptr(), "");
		tmp.sprintf("%u_User", i);
		p->m_szUser = cfg.readEntry(tmp.ptr(), "");
		tmp.sprintf("%u_Pass", i);
		p->m_szPass = cfg.readEntry(tmp.ptr(), "");

		tmp.sprintf("%u_Protocol", i);
		KviCString type = cfg.readEntry(tmp.ptr(), "SOCKSv4");
		p->setNamedProtocol(type.ptr());

		tmp.sprintf("%u_IsIPv6", i);
		p->m_bIsIPv6 = cfg.readBoolEntry(tmp.ptr(), false);
		tmp.sprintf("%u_Current", i);
		if(cfg.readBoolEntry(tmp.ptr(), false))
			m_pCurrentProxy = p.get();
		m_pProxyList.push_back(std::move(p));
	}

	if(!m_pCurrentProxy && !m_pProxyList.empty())
		m_pCurrentProxy = m_pProxyList.front().get();
}

void KviProxyDataBase::save(const QString & filename)
{
	KviConfigurationFile cfg(filename, KviConfigurationFile::Write);

	cfg.clear();

	cfg.writeEntry("Entries", static_cast<unsigned>(m_pProxyList.size()));

	int i = 0;

	for(auto & p : m_pProxyList)
	{
		KviCString tmp(KviCString::Format, "%u_Hostname", i);
		cfg.writeEntry(tmp.ptr(), p->m_szHostname);
		tmp.sprintf("%u_Port", i);
		cfg.writeEntry(tmp.ptr(), p->m_uPort);
		tmp.sprintf("%u_Ip", i);
		cfg.writeEntry(tmp.ptr(), p->m_szIp);
		tmp.sprintf("%u_User", i);
		cfg.writeEntry(tmp.ptr(), p->m_szUser);
		tmp.sprintf("%u_Pass", i);
		cfg.writeEntry(tmp.ptr(), p->m_szPass);

		tmp.sprintf("%u_Protocol", i);
		KviCString type;
		switch(p->m_protocol)
		{
			case KviProxy::Socks5:
				type = "SOCKSv5";
				break;
			case KviProxy::Http:
				type = "HTTP";
				break;
			default:
				type = "SOCKSv4";
				break;
		}
		cfg.writeEntry(tmp.ptr(), type.ptr());

		tmp.sprintf("%u_IsIPv6", i);
		cfg.writeEntry(tmp.ptr(), p->m_bIsIPv6);
		tmp.sprintf("%u_Current", i);
		if(m_pCurrentProxy == p.get())
			cfg.writeEntry(tmp.ptr(), true);
		i++;
	}
}
