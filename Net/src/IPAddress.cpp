//
// IPAddress.cpp
//
// $Id: //poco/1.4/Net/src/IPAddress.cpp#4 $
//
// Library: Net
// Package: NetCore
// Module:  IPAddress
//
// Copyright (c) 2005-2011, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/Net/IPAddress.h"
#include "Poco/Net/NetException.h"
#include "Poco/RefCountedObject.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Types.h"
#include <algorithm>
#include <cstring>


using Poco::RefCountedObject;
using Poco::NumberFormatter;
using Poco::UInt8;
using Poco::UInt16;
using Poco::UInt32;


namespace Poco {
namespace Net {


//
// IPAddressImpl
//


class IPAddressImpl: public RefCountedObject
{
public:
	virtual std::string toString() const = 0;
	virtual poco_socklen_t length() const = 0;
	virtual const void* addr() const = 0;
	virtual IPAddress::Family family() const = 0;
	virtual int af() const = 0;
	virtual Poco::UInt32 scope() const = 0;
	virtual bool isWildcard() const	= 0;
	virtual bool isBroadcast() const = 0;
	virtual bool isLoopback() const = 0;
	virtual bool isMulticast() const = 0;
	virtual bool isLinkLocal() const = 0;
	virtual bool isSiteLocal() const = 0;
	virtual bool isIPv4Mapped() const = 0;
	virtual bool isIPv4Compatible() const = 0;
	virtual bool isWellKnownMC() const = 0;
	virtual bool isNodeLocalMC() const = 0;
	virtual bool isLinkLocalMC() const = 0;
	virtual bool isSiteLocalMC() const = 0;
	virtual bool isOrgLocalMC() const = 0;
	virtual bool isGlobalMC() const = 0;
	virtual void mask(const IPAddressImpl* pMask, const IPAddressImpl* pSet) = 0;
	virtual IPAddressImpl* clone() const = 0;

protected:
	IPAddressImpl()
	{
#if defined(_WIN32)
		Poco::Net::initializeNetwork();
#endif
	}
	
	virtual ~IPAddressImpl()
	{
#if defined(_WIN32)
		Poco::Net::uninitializeNetwork();
#endif
	}

private:
	IPAddressImpl(const IPAddressImpl&);
	IPAddressImpl& operator = (const IPAddressImpl&);
};


class IPv4AddressImpl: public IPAddressImpl
{
public:
	IPv4AddressImpl()
	{
		std::memset(&_addr, 0, sizeof(_addr));
	}
	
	IPv4AddressImpl(const void* addr)
	{
		std::memcpy(&_addr, addr, sizeof(_addr));
	}
	
	std::string toString() const
	{
		const UInt8* bytes = reinterpret_cast<const UInt8*>(&_addr);
		std::string result;
		result.reserve(16);
		NumberFormatter::append(result, bytes[0]);
		result.append(".");
		NumberFormatter::append(result, bytes[1]);
		result.append(".");
		NumberFormatter::append(result, bytes[2]);
		result.append(".");
		NumberFormatter::append(result, bytes[3]);
		return result;
	}

	poco_socklen_t length() const
	{
		return sizeof(_addr);
	}
	
	const void* addr() const
	{
		return &_addr;
	}
	
	IPAddress::Family family() const
	{
		return IPAddress::IPv4;
	}
	
	int af() const
	{
		return AF_INET;
	}
	
	Poco::UInt32 scope() const
	{
		return 0;
	}
	
	bool isWildcard() const
	{
		return _addr.s_addr == INADDR_ANY;
	}
	
	bool isBroadcast() const
	{
		return _addr.s_addr == INADDR_NONE;
	}
	
	bool isLoopback() const
	{
		return (ntohl(_addr.s_addr) & 0xFF000000) == 0x7F000000; // 127.0.0.1 to 127.255.255.255
	}
	
	bool isMulticast() const
	{
		return (ntohl(_addr.s_addr) & 0xF0000000) == 0xE0000000; // 224.0.0.0/24 to 239.0.0.0/24
	}
		
	bool isLinkLocal() const
	{
		return (ntohl(_addr.s_addr) & 0xFFFF0000) == 0xA9FE0000; // 169.254.0.0/16
	}
	
	bool isSiteLocal() const
	{
		UInt32 addr = ntohl(_addr.s_addr);
		return (addr & 0xFF000000) == 0x0A000000 ||        // 10.0.0.0/24
		       (addr & 0xFFFF0000) == 0xC0A80000 ||        // 192.68.0.0/16
		       (addr >= 0xAC100000 && addr <= 0xAC1FFFFF); // 172.16.0.0 to 172.31.255.255
	}
	
	bool isIPv4Compatible() const
	{
		return true;
	}

	bool isIPv4Mapped() const
	{
		return true;
	}

	bool isWellKnownMC() const
	{
		return (ntohl(_addr.s_addr) & 0xFFFFFF00) == 0xE0000000; // 224.0.0.0/8
	}
	
	bool isNodeLocalMC() const
	{
		return false;
	}
	
	bool isLinkLocalMC() const
	{
		return (ntohl(_addr.s_addr) & 0xFF000000) == 0xE0000000; // 244.0.0.0/24
	}
	
	bool isSiteLocalMC() const
	{
		return (ntohl(_addr.s_addr) & 0xFFFF0000) == 0xEFFF0000; // 239.255.0.0/16
	}
	
	bool isOrgLocalMC() const
	{
		return (ntohl(_addr.s_addr) & 0xFFFF0000) == 0xEFC00000; // 239.192.0.0/16
	}
	
	bool isGlobalMC() const
	{
		UInt32 addr = ntohl(_addr.s_addr);
		return addr >= 0xE0000100 && addr <= 0xEE000000; // 224.0.1.0 to 238.255.255.255
	}

	static IPv4AddressImpl* parse(const std::string& addr)
	{
		if (addr.empty()) return 0;		
#if defined(_WIN32) 
		struct in_addr ia;
		ia.s_addr = inet_addr(addr.c_str());
		if (ia.s_addr == INADDR_NONE && addr != "255.255.255.255")
			return 0;
		else
			return new IPv4AddressImpl(&ia);
#else
#if __GNUC__ < 3 || defined(POCO_VXWORKS)
		struct in_addr ia;
		ia.s_addr = inet_addr(const_cast<char*>(addr.c_str()));
		if (ia.s_addr == INADDR_NONE && addr != "255.255.255.255")
			return 0;
		else
			return new IPv4AddressImpl(&ia);
#else
		struct in_addr ia;
		if (inet_aton(addr.c_str(), &ia))
			return new IPv4AddressImpl(&ia);
		else
			return 0;
#endif
#endif
	}
	
	void mask(const IPAddressImpl* pMask, const IPAddressImpl* pSet)
	{
		poco_assert (pMask->af() == AF_INET && pSet->af() == AF_INET);
		
		_addr.s_addr &= static_cast<const IPv4AddressImpl*>(pMask)->_addr.s_addr;
		_addr.s_addr |= static_cast<const IPv4AddressImpl*>(pSet)->_addr.s_addr & ~static_cast<const IPv4AddressImpl*>(pMask)->_addr.s_addr;
	}
	
	IPAddressImpl* clone() const
	{
		return new IPv4AddressImpl(&_addr);
	}
		
private:
	struct in_addr _addr;	
};


#if defined(POCO_HAVE_IPv6)


class IPv6AddressImpl: public IPAddressImpl
{
public:
	IPv6AddressImpl():
		_scope(0)
	{
		std::memset(&_addr, 0, sizeof(_addr));
	}

	IPv6AddressImpl(const void* addr):
		_scope(0)
	{
		std::memcpy(&_addr, addr, sizeof(_addr));
	}

	IPv6AddressImpl(const void* addr, Poco::UInt32 scope):
		_scope(scope)
	{
		std::memcpy(&_addr, addr, sizeof(_addr));
	}

	std::string toString() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		if (isIPv4Compatible() || isIPv4Mapped())
		{
			std::string result;
			result.reserve(24);
			if (words[5] == 0)
				result.append("::");
			else
				result.append("::FFFF:");
			const UInt8* bytes = reinterpret_cast<const UInt8*>(&_addr);
			NumberFormatter::append(result, bytes[12]);
			result.append(".");
			NumberFormatter::append(result, bytes[13]);
			result.append(".");
			NumberFormatter::append(result, bytes[14]);
			result.append(".");
			NumberFormatter::append(result, bytes[15]);
			return result;
		}
		else
		{
			std::string result;
			result.reserve(64);
			bool zeroSequence = false;
			int i = 0;
			while (i < 8)
			{
				if (!zeroSequence && words[i] == 0)
				{
					int zi = i;
					while (zi < 8 && words[zi] == 0) ++zi;
					if (zi > i + 1)
					{
						i = zi;
						result.append(":");
						zeroSequence = true;
					}
				}
				if (i > 0) result.append(":");
				if (i < 8) NumberFormatter::appendHex(result, ntohs(words[i++]));
			}
			if (_scope > 0)
			{
				result.append("%");
#if defined(_WIN32)
				NumberFormatter::append(result, _scope);
#else
				char buffer[IFNAMSIZ];
				if (if_indextoname(_scope, buffer))
				{
					result.append(buffer);
				}
				else
				{
					NumberFormatter::append(result, _scope);
				}
#endif
			}
			return result;
		}
	}
	
	poco_socklen_t length() const
	{
		return sizeof(_addr);
	}

	const void* addr() const
	{
		return &_addr;
	}

	IPAddress::Family family() const
	{
		return IPAddress::IPv6;
	}

	int af() const
	{
		return AF_INET6;
	}
	
	Poco::UInt32 scope() const
	{
		return _scope;
	}

	bool isWildcard() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0 && 
		       words[4] == 0 && words[5] == 0 && words[6] == 0 && words[7] == 0;
	}
	
	bool isBroadcast() const
	{
		return false;
	}
	
	bool isLoopback() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0 && 
		       words[4] == 0 && words[5] == 0 && words[6] == 0 && words[7] == 0x0100;
	}
	
	bool isMulticast() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (words[0] & 0xE0FF) == 0x00FF;
	}
		
	bool isLinkLocal() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (words[0] & 0xE0FF) == 0x80FE;
	}
	
	bool isSiteLocal() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (words[0] & 0xE0FF) == 0xC0FE;
	}
	
	bool isIPv4Compatible() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0 && words[4] == 0 && words[5] == 0;
	}

	bool isIPv4Mapped() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0 && words[4] == 0 && words[5] == 0xFFFF;
	}

	bool isWellKnownMC() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (words[0] & 0xF0FF) == 0x00FF;
	}
	
	bool isNodeLocalMC() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (words[0] & 0xEFFF) == 0x01FF;
	}
	
	bool isLinkLocalMC() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (words[0] & 0xEFFF) == 0x02FF;
	}
	
	bool isSiteLocalMC() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (words[0] & 0xEFFF) == 0x05FF;
	}
	
	bool isOrgLocalMC() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (words[0] & 0xEFFF) == 0x08FF;
	}
	
	bool isGlobalMC() const
	{
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (words[0] & 0xEFFF) == 0x0FFF;
	}

	static IPv6AddressImpl* parse(const std::string& addr)
	{
		if (addr.empty()) return 0;
#if defined(_WIN32)
		struct addrinfo* pAI;
		struct addrinfo hints;
		std::memset(&hints, 0, sizeof(hints));
		hints.ai_flags = AI_NUMERICHOST;
		int rc = getaddrinfo(addr.c_str(), NULL, &hints, &pAI);
		if (rc == 0)
		{
			IPv6AddressImpl* pResult = new IPv6AddressImpl(&reinterpret_cast<struct sockaddr_in6*>(pAI->ai_addr)->sin6_addr, static_cast<int>(reinterpret_cast<struct sockaddr_in6*>(pAI->ai_addr)->sin6_scope_id));
			freeaddrinfo(pAI);
			return pResult;
		}
		else return 0;
#else
		struct in6_addr ia;
		std::string::size_type pos = addr.find('%');
		if (std::string::npos != pos)
		{
			std::string::size_type start = ('[' == addr[0]) ? 1 : 0;
			std::string unscopedAddr(addr, start, pos - start);
			std::string scope(addr, pos + 1, addr.size() - start - pos);
			Poco::UInt32 scopeId(0);
			if (!(scopeId = if_nametoindex(scope.c_str())))
				return 0;
			if (inet_pton(AF_INET6, unscopedAddr.c_str(), &ia) == 1)
				return new IPv6AddressImpl(&ia, scopeId);
			else
				return 0;
		}
		else
		{
			if (inet_pton(AF_INET6, addr.c_str(), &ia) == 1)
				return new IPv6AddressImpl(&ia);
			else
				return 0;
		}
#endif
	}
	
	void mask(const IPAddressImpl* pMask, const IPAddressImpl* pSet)
	{
		throw Poco::NotImplementedException("mask() is only supported for IPv4 addresses");
	}

	IPAddressImpl* clone() const
	{
		return new IPv6AddressImpl(&_addr, _scope);
	}

private:
	struct in6_addr _addr;	
	Poco::UInt32    _scope;
};


#endif // POCO_HAVE_IPv6


//
// IPAddress
//


IPAddress::IPAddress(): _pImpl(new IPv4AddressImpl)
{
}


IPAddress::IPAddress(const IPAddress& addr): _pImpl(addr._pImpl)
{
	_pImpl->duplicate();
}


IPAddress::IPAddress(Family family): _pImpl(0)
{
	if (family == IPv4)
		_pImpl = new IPv4AddressImpl();
#if defined(POCO_HAVE_IPv6)
	else if (family == IPv6)
		_pImpl = new IPv6AddressImpl();
#endif
	else
		throw Poco::InvalidArgumentException("Invalid or unsupported address family passed to IPAddress()");
}


IPAddress::IPAddress(const std::string& addr)
{
	_pImpl = IPv4AddressImpl::parse(addr);
#if defined(POCO_HAVE_IPv6)
	if (!_pImpl)
		_pImpl = IPv6AddressImpl::parse(addr);
#endif
	if (!_pImpl) throw InvalidAddressException(addr);
}


IPAddress::IPAddress(const std::string& addr, Family family): _pImpl(0)
{
	if (family == IPv4)
		_pImpl = IPv4AddressImpl::parse(addr);
#if defined(POCO_HAVE_IPv6)
	else if (family == IPv6)
		_pImpl = IPv6AddressImpl::parse(addr);
#endif
	else throw Poco::InvalidArgumentException("Invalid or unsupported address family passed to IPAddress()");
	if (!_pImpl) throw InvalidAddressException(addr);
}


IPAddress::IPAddress(const void* addr, poco_socklen_t length)
{
	if (length == sizeof(struct in_addr))
		_pImpl = new IPv4AddressImpl(addr);
#if defined(POCO_HAVE_IPv6)
	else if (length == sizeof(struct in6_addr))
		_pImpl = new IPv6AddressImpl(addr);
#endif
	else throw Poco::InvalidArgumentException("Invalid address length passed to IPAddress()");
}


IPAddress::IPAddress(const void* addr, poco_socklen_t length, Poco::UInt32 scope)
{
	if (length == sizeof(struct in_addr))
		_pImpl = new IPv4AddressImpl(addr);
#if defined(POCO_HAVE_IPv6)
	else if (length == sizeof(struct in6_addr))
		_pImpl = new IPv6AddressImpl(addr, scope);
#endif
	else throw Poco::InvalidArgumentException("Invalid address length passed to IPAddress()");
}


IPAddress::~IPAddress()
{
	_pImpl->release();
}


IPAddress& IPAddress::operator = (const IPAddress& addr)
{
	if (&addr != this)
	{
		_pImpl->release();
		_pImpl = addr._pImpl;
		_pImpl->duplicate();
	}
	return *this;
}


void IPAddress::swap(IPAddress& address)
{
	std::swap(_pImpl, address._pImpl);
}

	
IPAddress::Family IPAddress::family() const
{
	return _pImpl->family();
}


Poco::UInt32 IPAddress::scope() const
{
	return _pImpl->scope();
}

	
std::string IPAddress::toString() const
{
	return _pImpl->toString();
}


bool IPAddress::isWildcard() const
{
	return _pImpl->isWildcard();
}
	
bool IPAddress::isBroadcast() const
{
	return _pImpl->isBroadcast();
}


bool IPAddress::isLoopback() const
{
	return _pImpl->isLoopback();
}


bool IPAddress::isMulticast() const
{
	return _pImpl->isMulticast();
}

	
bool IPAddress::isUnicast() const
{
	return !isWildcard() && !isBroadcast() && !isMulticast();
}

	
bool IPAddress::isLinkLocal() const
{
	return _pImpl->isLinkLocal();
}


bool IPAddress::isSiteLocal() const
{
	return _pImpl->isSiteLocal();
}


bool IPAddress::isIPv4Compatible() const
{
	return _pImpl->isIPv4Compatible();
}


bool IPAddress::isIPv4Mapped() const
{
	return _pImpl->isIPv4Mapped();
}


bool IPAddress::isWellKnownMC() const
{
	return _pImpl->isWellKnownMC();
}


bool IPAddress::isNodeLocalMC() const
{
	return _pImpl->isNodeLocalMC();
}


bool IPAddress::isLinkLocalMC() const
{
	return _pImpl->isLinkLocalMC();
}


bool IPAddress::isSiteLocalMC() const
{
	return _pImpl->isSiteLocalMC();
}


bool IPAddress::isOrgLocalMC() const
{
	return _pImpl->isOrgLocalMC();
}


bool IPAddress::isGlobalMC() const
{
	return _pImpl->isGlobalMC();
}


bool IPAddress::operator == (const IPAddress& a) const
{
	poco_socklen_t l1 = length();
	poco_socklen_t l2 = a.length();
	if (l1 == l2)
		return std::memcmp(addr(), a.addr(), l1) == 0;
	else
		return false;
}


bool IPAddress::operator != (const IPAddress& a) const
{
	poco_socklen_t l1 = length();
	poco_socklen_t l2 = a.length();
	if (l1 == l2)
		return std::memcmp(addr(), a.addr(), l1) != 0;
	else
		return true;
}


bool IPAddress::operator < (const IPAddress& a) const
{
	poco_socklen_t l1 = length();
	poco_socklen_t l2 = a.length();
	if (l1 == l2)
		return std::memcmp(addr(), a.addr(), l1) < 0;
	else
		return l1 < l2;
}


bool IPAddress::operator <= (const IPAddress& a) const
{
	poco_socklen_t l1 = length();
	poco_socklen_t l2 = a.length();
	if (l1 == l2)
		return std::memcmp(addr(), a.addr(), l1) <= 0;
	else
		return l1 < l2;
}


bool IPAddress::operator > (const IPAddress& a) const
{
	poco_socklen_t l1 = length();
	poco_socklen_t l2 = a.length();
	if (l1 == l2)
		return std::memcmp(addr(), a.addr(), l1) > 0;
	else
		return l1 > l2;
}


bool IPAddress::operator >= (const IPAddress& a) const
{
	poco_socklen_t l1 = length();
	poco_socklen_t l2 = a.length();
	if (l1 == l2)
		return std::memcmp(addr(), a.addr(), l1) >= 0;
	else
		return l1 > l2;
}


poco_socklen_t IPAddress::length() const
{
	return _pImpl->length();
}

	
const void* IPAddress::addr() const
{
	return _pImpl->addr();
}


int IPAddress::af() const
{
	return _pImpl->af();
}


void IPAddress::init(IPAddressImpl* pImpl)
{
	_pImpl->release();
	_pImpl = pImpl;
}


IPAddress IPAddress::parse(const std::string& addr)
{
	return IPAddress(addr);
}


bool IPAddress::tryParse(const std::string& addr, IPAddress& result)
{
	IPAddressImpl* pImpl = IPv4AddressImpl::parse(addr);
#if defined(POCO_HAVE_IPv6)
	if (!pImpl) pImpl = IPv6AddressImpl::parse(addr);
#endif
	if (pImpl)
	{
		result.init(pImpl);
		return true;
	}
	else return false;
}


void IPAddress::mask(const IPAddress& mask)
{
	IPAddressImpl* pClone = _pImpl->clone();
	_pImpl->release();
	_pImpl = pClone;
	IPAddress null;
	_pImpl->mask(mask._pImpl, null._pImpl);
}


void IPAddress::mask(const IPAddress& mask, const IPAddress& set)
{
	IPAddressImpl* pClone = _pImpl->clone();
	_pImpl->release();
	_pImpl = pClone;
	_pImpl->mask(mask._pImpl, set._pImpl);
}


IPAddress IPAddress::wildcard(Family family)
{
	return IPAddress(family);
}


IPAddress IPAddress::broadcast()
{
	struct in_addr ia;
	ia.s_addr = INADDR_NONE;
	return IPAddress(&ia, sizeof(ia));
}


} } // namespace Poco::Net
