/*
    PowerDNS Versatile Database Driven Nameserver
    Copyright (C) 2002 - 2011 PowerDNS.COM BV

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation

    Additionally, the license of this program contains a special
    exception which allows to distribute the program in binary form when
    it is linked against OpenSSL.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
// $Id$ 
/* (C) 2002 POWERDNS.COM BV */
#pragma once
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include "qtype.hh"
#include "dnsname.hh"
#include <time.h>
#include <sys/types.h>
class DNSBackend;
class DNSName; // FIXME400

struct SOAData
{
  SOAData() : ttl(0), serial(0), refresh(0), retry(0), expire(0), default_ttl(0), db(0), domain_id(-1), scopeMask(0) {};

  DNSName qname;
  DNSName nameserver;
  DNSName hostmaster;
  uint32_t ttl;
  uint32_t serial;
  uint32_t refresh;
  uint32_t retry;
  uint32_t expire;
  uint32_t default_ttl;
  DNSBackend *db;
  int domain_id;
  uint8_t scopeMask;
};

class RCode
{
public:
  enum rcodes_ { NoError=0, FormErr=1, ServFail=2, NXDomain=3, NotImp=4, Refused=5, YXDomain=6, YXRRSet=7, NXRRSet=8, NotAuth=9, NotZone=10};
  static std::string to_s(unsigned short rcode);
  static std::vector<std::string> rcodes_s;
};

class Opcode
{
public:
  enum { Query=0, IQuery=1, Status=2, Notify=4, Update=5 };
};

//! This class represents a resource record
class DNSResourceRecord
{
public:
  DNSResourceRecord() : last_modified(0), ttl(0), signttl(0), domain_id(-1), qclass(1), d_place(ANSWER), scopeMask(0), auth(1), disabled(0) {};
  explicit DNSResourceRecord(const struct DNSRecord&);
  ~DNSResourceRecord(){};

  enum Place : uint8_t {QUESTION=0, ANSWER=1, AUTHORITY=2, ADDITIONAL=3}; //!< Type describing the positioning of a DNSResourceRecord within, say, a DNSPacket

  void setContent(const string& content);
  string getZoneRepresentation() const;

  // data
  DNSName qname; //!< the name of this record, for example: www.powerdns.com
  DNSName wildcardname;
  string content; //!< what this record points to. Example: 10.1.2.3

  // Aligned on 8-byte boundries on systems where time_t is 8 bytes and int
  // is 4 bytes, aka modern linux on x86_64
  time_t last_modified; //!< For autocalculating SOA serial numbers - the backend needs to fill this in

  uint32_t ttl; //!< Time To Live of this record
  uint32_t signttl; //!< If non-zero, use this TTL as original TTL in the RRSIG

  int domain_id; //!< If a backend implements this, the domain_id of the zone this record is in
  QType qtype; //!< qtype of this record, ie A, CNAME, MX etc
  uint16_t qclass; //!< class of this record

  Place d_place; //!< This specifies where a record goes within the packet
  uint8_t scopeMask;
  bool auth;
  bool disabled;

  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & qtype;
    ar & qclass;
    ar & qname;
    ar & wildcardname;
    ar & content;
    ar & ttl;
    ar & domain_id;
    ar & last_modified;
    ar & d_place;
    ar & auth;
    ar & disabled;
  }

  bool operator==(const DNSResourceRecord& rhs);

  bool operator<(const DNSResourceRecord &b) const
  {
    if(qname < b.qname)
      return true;
    if(qname == b.qname)
      return(content < b.content);
    return false;
  }
};

#define GCCPACKATTRIBUTE __attribute__((packed))

struct dnsrecordheader
{
  uint16_t d_type;
  uint16_t d_class;
  uint32_t d_ttl;
  uint16_t d_clen;
} GCCPACKATTRIBUTE;

struct EDNS0Record 
{ 
        uint8_t extRCode, version; 
        uint16_t Z; 
} GCCPACKATTRIBUTE;

#if defined(__FreeBSD__) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
#include <machine/endian.h>
#elif __linux__ || __GNU__
# include <endian.h>

#else  // with thanks to <arpa/nameser.h> 

# define LITTLE_ENDIAN   1234    /* least-significant byte first (vax, pc) */
# define BIG_ENDIAN      4321    /* most-significant byte first (IBM, net) */
# define PDP_ENDIAN      3412    /* LSB first in word, MSW first in long (pdp) */

#if defined(vax) || defined(ns32000) || defined(sun386) || defined(i386) || \
        defined(__i386) || defined(__ia64) || defined(__amd64) || \
        defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
        defined(__alpha__) || defined(__alpha) || \
        (defined(__Lynx__) && defined(__x86__))
# define BYTE_ORDER      LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(__sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined(__hp3000s900) || defined(MPE) || \
    defined(BIT_ZERO_ON_LEFT) || defined(m68k) || \
        (defined(__Lynx__) && \
        (defined(__68k__) || defined(__sparc__) || defined(__powerpc__)))
# define BYTE_ORDER      BIG_ENDIAN
#endif

#endif

struct dnsheader {
        unsigned        id :16;         /* query identification number */
#if BYTE_ORDER == BIG_ENDIAN
                        /* fields in third byte */
        unsigned        qr: 1;          /* response flag */
        unsigned        opcode: 4;      /* purpose of message */
        unsigned        aa: 1;          /* authoritative answer */
        unsigned        tc: 1;          /* truncated message */
        unsigned        rd: 1;          /* recursion desired */
                        /* fields in fourth byte */
        unsigned        ra: 1;          /* recursion available */
        unsigned        unused :1;      /* unused bits (MBZ as of 4.9.3a3) */
        unsigned        ad: 1;          /* authentic data from named */
        unsigned        cd: 1;          /* checking disabled by resolver */
        unsigned        rcode :4;       /* response code */
#elif BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
                        /* fields in third byte */
        unsigned        rd :1;          /* recursion desired */
        unsigned        tc :1;          /* truncated message */
        unsigned        aa :1;          /* authoritative answer */
        unsigned        opcode :4;      /* purpose of message */
        unsigned        qr :1;          /* response flag */
                        /* fields in fourth byte */
        unsigned        rcode :4;       /* response code */
        unsigned        cd: 1;          /* checking disabled by resolver */
        unsigned        ad: 1;          /* authentic data from named */
        unsigned        unused :1;      /* unused bits (MBZ as of 4.9.3a3) */
        unsigned        ra :1;          /* recursion available */
#endif
                        /* remaining bytes */
        unsigned        qdcount :16;    /* number of question entries */
        unsigned        ancount :16;    /* number of answer entries */
        unsigned        nscount :16;    /* number of authority entries */
        unsigned        arcount :16;    /* number of resource entries */
};

inline uint16_t * getFlagsFromDNSHeader(struct dnsheader * dh)
{
  return (uint16_t*) (((char *) dh) + sizeof(uint16_t));
}

#if BYTE_ORDER == BIG_ENDIAN
#define FLAGS_RD_OFFSET (8)
#define FLAGS_CD_OFFSET (12)
#elif BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
#define FLAGS_RD_OFFSET (0)
#define FLAGS_CD_OFFSET (12)
#endif

#define L theL()
extern time_t s_starttime;

uint32_t hashQuestion(const char* packet, uint16_t len, uint32_t init);

//! compares two dns packets in canonical order, skipping the header, but including the question and the qtype
inline bool dnspacketLessThan(const std::string& a, const std::string& b)
{
  /* BEFORE YOU ATTEMPT TO MERGE THIS WITH DNSNAME::CANONICALCOMPARE! 
     Please note that that code is subtly different, and for example only has
     to deal with a string of labels, and not a trailing packet. Also, the comparison
     rules are different since we also have to take into account qname and qtype.
     So just grin and bear it.
   */

  if(a.length() <= 12 || b.length() <= 12) 
    return a.length() < b.length();

  uint8_t ourpos[64], rhspos[64];
  uint8_t ourcount=0, rhscount=0;
  //cout<<"Asked to compare "<<toString()<<" to "<<rhs.toString()<<endl;
  const unsigned char* p;
  for(p = (const unsigned char*)a.c_str()+12; p < (const unsigned char*)a.c_str() + a.size() && *p && ourcount < sizeof(ourpos); p+=*p+1)
    ourpos[ourcount++]=(p-(const unsigned char*)a.c_str());
  if(p>=(const unsigned char*)a.c_str() + a.size())
    return true;

  uint16_t aQtype = *(p+1)*256 + *(p+2);
  uint16_t aQclass =*(p+3)*256 + *(p+4);

  for(p = (const unsigned char*)b.c_str()+12; p < (const unsigned char*)b.c_str() + b.size() && *p && rhscount < sizeof(rhspos); p+=*p+1)
    rhspos[rhscount++]=(p-(const unsigned char*)b.c_str());

  if(p>=(const unsigned char*)b.c_str() + b.size())
    return false;

  uint16_t bQtype = *(p+1)*256 + *(p+2);
  uint16_t bQclass =*(p+3)*256 + *(p+4);

  if(ourcount == sizeof(ourpos) || rhscount==sizeof(rhspos)) {
    DNSName aname(a.c_str(), a.size(), 12, false, &aQtype, &aQclass);
    DNSName bname(b.c_str(), b.size(), 12, false, &bQtype, &bQclass);

    if(aname.slowCanonCompare(bname))
      return true;
    if(aname!=bname)
      return false;
    return boost::tie(aQtype, aQclass) < boost::tie(bQtype, bQclass);
  }
  for(;;) {
    if(ourcount == 0 && rhscount != 0)
      return true;
    if(ourcount == 0 && rhscount == 0)
      break;
    if(ourcount !=0 && rhscount == 0)
      return false;
    ourcount--;
    rhscount--;

    bool res=std::lexicographical_compare(
					  a.c_str() + ourpos[ourcount] + 1, 
					  a.c_str() + ourpos[ourcount] + 1 + *(a.c_str() + ourpos[ourcount]),
					  b.c_str() + rhspos[rhscount] + 1, 
					  b.c_str() + rhspos[rhscount] + 1 + *(b.c_str() + rhspos[rhscount]),
					  [](const char& a, const char& b) {
					    return dns2_tolower(a) < dns2_tolower(b); 
					  });
    
    //    cout<<"Forward: "<<res<<endl;
    if(res)
      return true;

    res=std::lexicographical_compare(	  b.c_str() + rhspos[rhscount] + 1, 
					  b.c_str() + rhspos[rhscount] + 1 + *(b.c_str() + rhspos[rhscount]),
					  a.c_str() + ourpos[ourcount] + 1, 
					  a.c_str() + ourpos[ourcount] + 1 + *(a.c_str() + ourpos[ourcount]),
					  [](const char& a, const char& b) {
					    return dns2_tolower(a) < dns2_tolower(b); 
					  });
    //    cout<<"Reverse: "<<res<<endl;
    if(res)
      return false;
  }
  
  return boost::tie(aQtype, aQclass) < boost::tie(bQtype, bQclass);
}


/** for use by DNSPacket, converts a SOAData class to a ascii line again */
string serializeSOAData(const SOAData &data);
string &attodot(string &str);  //!< for when you need to insert an email address in the SOA

vector<DNSResourceRecord> convertRRS(const vector<DNSRecord>& in);
