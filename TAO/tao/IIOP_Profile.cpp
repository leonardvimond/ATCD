// This may look like C, but it's really -*- C++ -*-
// $Id$

#include "tao/IIOP_Profile.h"
#include "tao/IIOP_Connect.h"
#include "tao/GIOP.h"
#include "tao/CDR.h"
#include "tao/Environment.h"
#include "tao/ORB.h"
#include "tao/ORB_Core.h"
#include "tao/POA.h"
#include "tao/debug.h"

ACE_RCSID(tao, IIOP_Profile, "$Id$")

#if !defined (__ACE_INLINE__)
# include "tao/IIOP_Profile.i"
#endif /* __ACE_INLINE__ */

static const char *prefix_ = "iiop:";

const char TAO_IIOP_Profile::object_key_delimiter = '/';

TAO_IIOP_Profile::TAO_IIOP_Profile (const ACE_INET_Addr& addr,
                                    const char *object_key)
  : TAO_Profile (TAO_IOP_TAG_INTERNET_IOP),
    host_ (0),
    port_ (0),
    version_ (DEF_IIOP_MAJOR, DEF_IIOP_MINOR),
    object_key_ (),
    object_addr_ (addr),
    hint_ (0)
{
  this->set (addr);
  int l = ACE_OS::strlen (object_key);
  this->object_key_.length (l);

  for (int i = 0; i < l; ++i)
    this->object_key_[i] = object_key[i];

}

TAO_IIOP_Profile::TAO_IIOP_Profile (const ACE_INET_Addr& addr,
                                    const TAO_ObjectKey& object_key)
  : TAO_Profile (TAO_IOP_TAG_INTERNET_IOP),
    host_ (0),
    port_ (0),
    version_ (DEF_IIOP_MAJOR, DEF_IIOP_MINOR),
    object_key_ (object_key),
    object_addr_ (addr),
    hint_ (0)
{
  this->set (addr);
}

TAO_IIOP_Profile::TAO_IIOP_Profile (const ACE_INET_Addr& addr,
                                    const TAO_IOP_Version& version,
                                    const char *object_key)
  : TAO_Profile (TAO_IOP_TAG_INTERNET_IOP),
    host_ (0),
    port_ (0),
    version_ (version),
    object_key_ (),
    object_addr_ (addr),
    hint_ (0)
{
  this->set (addr);
  int l = ACE_OS::strlen (object_key);
  this->object_key_.length (l);

  for (int i = 0; i < l; ++i)
    this->object_key_[i] = object_key[i];

}

TAO_IIOP_Profile::TAO_IIOP_Profile (const ACE_INET_Addr& addr,
                                    const TAO_IOP_Version& version,
                                    const TAO_ObjectKey& object_key)
  : TAO_Profile (TAO_IOP_TAG_INTERNET_IOP),
    host_ (0),
    port_ (0),
    version_ (version),
    object_key_ (object_key),
    object_addr_ (addr),
    hint_ (0)
{
  this->set (addr);
}

TAO_IIOP_Profile::TAO_IIOP_Profile (const char* host,
                                    CORBA::UShort port,
                                    const TAO_ObjectKey& object_key)
  : TAO_Profile (TAO_IOP_TAG_INTERNET_IOP),
    host_ (0),
    port_ (port),
    version_ (DEF_IIOP_MAJOR, DEF_IIOP_MINOR),
    object_key_ (object_key),
    object_addr_ (port, host),
    hint_ (0)
{

  if (host)
  {
    ACE_NEW (this->host_,
             char[ACE_OS::strlen (host) + 1]);
    ACE_OS::strcpy (this->host_, host);
  }

}

TAO_IIOP_Profile::TAO_IIOP_Profile (const char* host,
                                    CORBA::UShort port,
                                    const TAO_ObjectKey& object_key,
                                    const ACE_INET_Addr& addr)
  : TAO_Profile (TAO_IOP_TAG_INTERNET_IOP),
    host_ (0),
    port_ (port),
    version_ (DEF_IIOP_MAJOR, DEF_IIOP_MINOR),
    object_key_ (object_key),
    object_addr_ (addr),
    hint_ (0)
{

  if (host)
  {
    ACE_NEW (this->host_,
             char[ACE_OS::strlen (host) + 1]);
    ACE_OS::strcpy (this->host_, host);
  }

}

TAO_IIOP_Profile::TAO_IIOP_Profile (const char* host,
                                    CORBA::UShort port,
                                    const TAO_IOP_Version& version,
                                    const TAO_ObjectKey& object_key)
  : TAO_Profile (TAO_IOP_TAG_INTERNET_IOP),
    host_ (0),
    port_ (port),
    version_ (DEF_IIOP_MAJOR, DEF_IIOP_MINOR),
    object_key_ (object_key),
    object_addr_ (port, host),
    hint_ (0)
{
  ACE_UNUSED_ARG (version);

  ACE_NEW (this->host_,
           char[ACE_OS::strlen (host) + 1]);
  ACE_OS::strcpy (this->host_, host);

}

TAO_IIOP_Profile::TAO_IIOP_Profile (const TAO_IIOP_Profile *pfile)
  : TAO_Profile (pfile->tag ()),
    host_(0),
    port_(pfile->port_),
    version_(pfile->version_),
    object_key_(pfile->object_key_),
    object_addr_(pfile->object_addr_),
    hint_(0)
{

  ACE_NEW (this->host_,
           char[ACE_OS::strlen (pfile->host_) + 1]);
  ACE_OS::strcpy (this->host_, pfile->host_);
  hint_ = pfile->hint_;

}

TAO_IIOP_Profile::TAO_IIOP_Profile (const TAO_IIOP_Profile &pfile)
  : TAO_Profile (pfile.tag ()),
    host_(0),
    port_(pfile.port_),
    version_(pfile.version_),
    object_key_(pfile.object_key_),
    object_addr_(pfile.object_addr_),
    hint_(0)
{

  ACE_NEW (this->host_,
           char[ACE_OS::strlen (pfile.host_) + 1]);
  ACE_OS::strcpy (this->host_, pfile.host_);
  hint_ = pfile.hint_;

}

TAO_IIOP_Profile::TAO_IIOP_Profile (const TAO_IOP_Version &version)
  : TAO_Profile (TAO_IOP_TAG_INTERNET_IOP),
    host_ (0),
    port_ (0),
    version_ (version),
    object_key_ (),
    object_addr_ (),
    hint_ (0)
{
}

TAO_IIOP_Profile::TAO_IIOP_Profile (const char *string,
                                    CORBA::Environment &env)
  : TAO_Profile (TAO_IOP_TAG_INTERNET_IOP),
    host_ (0),
    port_ (0),
    version_ (DEF_IIOP_MAJOR, DEF_IIOP_MINOR),
    object_key_ (),
    object_addr_ (),
    hint_ (0)
{
  parse_string (string, env);
}

TAO_IIOP_Profile::TAO_IIOP_Profile (void)
  : TAO_Profile (TAO_IOP_TAG_INTERNET_IOP),
    host_ (0),
    port_ (0),
    version_ (DEF_IIOP_MAJOR, DEF_IIOP_MINOR),
    object_key_ (),
    object_addr_ (),
    hint_ (0)
{
}

int
TAO_IIOP_Profile::set (const ACE_INET_Addr& addr)
{
  char temphost[MAXHOSTNAMELEN + 1];
  const char *temphost2 = 0;

  this->port_ = addr.get_port_number();

  // @@ We really should be passing in TAO_ORB_Core as a parameter,
  // instead of using the ORB_Core singleton.
  if (TAO_ORB_Core_instance ()->orb_params ()->use_dotted_decimal_addresses ())
    {
      temphost2 = addr.get_host_addr ();
      if (temphost2 == 0)
        return -1;
    }
  else
    {
      if (addr.get_host_name (temphost,
                              sizeof temphost) != 0)
        return -1;

      temphost2 = temphost;
    }

  ACE_NEW_RETURN (this->host_,
                  char[ACE_OS::strlen (temphost2) + 1],
                  -1);
  ACE_OS::strcpy (this->host_, temphost2);

  return 0;

}

TAO_IIOP_Profile::~TAO_IIOP_Profile (void)
{
  delete [] this->host_;
  this->host_ = 0;
}

// return codes:
// -1 -> error
//  0 -> can't understand this version
//  1 -> success.
int
TAO_IIOP_Profile::decode (TAO_InputCDR& cdr)
{
  CORBA::ULong encap_len = cdr.length ();

  // Read and verify major, minor versions, ignoring IIOP
  // profiles whose versions we don't understand.
  //
  // @@ Fred: if we find a version like 1.5 we are supposed to handle
  // it, i.e. read the fields we know about and ignore the rest!
  //
  // XXX this doesn't actually go back and skip the whole
  // encapsulation...
  if (!(cdr.read_octet (this->version_.major)
        && this->version_.major == TAO_IIOP_Profile::DEF_IIOP_MAJOR
        && cdr.read_octet (this->version_.minor)
        && this->version_.minor <= TAO_IIOP_Profile::DEF_IIOP_MINOR))
  {
    if (TAO_debug_level > 0)
      {
        ACE_DEBUG ((LM_DEBUG,
                    "TAO (%P|%t) IIOP_Profile::decode - v%d.%d\n",
                    this->version_.major,
                    this->version_.minor));
      }
  }

  if (this->host_)
    {
      delete [] this->host_;
      this->host_ = 0;
    }

  // Get host and port
  if (cdr.read_string (this->host_) == 0
      || cdr.read_ushort (this->port_) == 0)
    {
      if (TAO_debug_level > 0)
        {
          ACE_DEBUG ((LM_DEBUG,
                      "TAO (%P|%t) IIOP_Profile::decode - "
                      "error while decoding host/port"));
        }
      return -1;
    }

  this->object_addr_.set (this->port_, this->host_);

  // ... and object key.

  if ((cdr >> this->object_key_) == 0)
    return -1;

  if (cdr.length () != 0 && TAO_debug_level)
    {
      // If there is extra data in the profile we are supposed to
      // ignore it, but print a warning just in case...
      ACE_DEBUG ((LM_DEBUG,
                  "%d bytes out of %d left after IIOP profile data\n",
                  cdr.length (),
                  encap_len));
    }
  if (cdr.good_bit ())
    return 1;

  return -1;
}

int
TAO_IIOP_Profile::parse_string (const char *string,
                                CORBA::Environment &ACE_TRY_ENV)
{
  if (!string || !*string)
    return 0;

  // Remove the "N.n@" version prefix, if it exists, and verify the
  // version is one that we accept.

  // Check for version
  if (isdigit (string [0]) &&
      string[1] == '.' &&
      isdigit (string [2]) &&
      string[3] == '@')
    {
      // @@ This may fail for non-ascii character sets [but take that
      // with a grain of salt]
      this->version_.set_version ((char) (string [0] - '0'),
                                  (char) (string [2] - '0'));
      string += 4;
      // Skip over the "N.n@"
    }

  if (this->version_.major != TAO_IIOP_Profile::DEF_IIOP_MAJOR ||
      this->version_.minor >  TAO_IIOP_Profile::DEF_IIOP_MINOR)
    {
      ACE_THROW_RETURN (CORBA::MARSHAL (), -1);
    }

  // Pull off the "hostname:port/" part of the objref
  // Copy the string because we are going to modify it...
  CORBA::String_var copy = CORBA::string_dup (string);

  char *start = copy.inout ();
  char *cp = ACE_OS::strchr (start, ':');

  if (cp == 0)
    {
      ACE_THROW_RETURN (CORBA::MARSHAL (), -1);
    }

  if (this->host_)
    {
      // @@ You are setting this->host_ using CORBA::string_alloc() a
      // couple of lines below, you should then use CORBA::string_free()
      // to release it!  In general use a single form of memory
      // allocation for a field/variable to avoid new/free() and
      // malloc/delete() mismatches.
      // Ohh, and if you are going to use CORBA::string_alloc() &
      // friends you may consider using CORBA::String_var to manage
      // the memory automatically (though there may be forces that
      // suggest otherwise).
      delete [] this->host_;
      this->host_ = 0;
    }

  this->host_ = CORBA::string_alloc (1 + cp - start);
  for (cp = this->host_; *start != ':'; *cp++ = *start++)
    continue;

  *cp = 0; start++; // increment past :

  cp = ACE_OS::strchr (start, this->object_key_delimiter);

  if (cp == 0)
    {
      CORBA::string_free (this->host_);
      ACE_THROW_RETURN (CORBA::MARSHAL (), -1);
    }

  this->port_ = (CORBA::UShort) ACE_OS::atoi (start);
  // @@ This call to atoi appears to pass in a string that
  //    still has the object key appended to it.
  //    Shouldn't we actually parse the port from the string
  //    rather than pass a `port/object_key' combined string?
  //                      -Ossama

  this->object_addr_.set (this->port_, this->host_);

  start = ++cp;  // increment past the /

  TAO_POA::decode_string_to_sequence (this->object_key_, start);

  return 1;
}

CORBA::Boolean
TAO_IIOP_Profile::is_equivalent (TAO_Profile *other_profile,
                                 CORBA::Environment &env)
{
  env.clear ();

  if (other_profile->tag () != TAO_IOP_TAG_INTERNET_IOP)
    return 0;

  TAO_IIOP_Profile *op =
    ACE_dynamic_cast (TAO_IIOP_Profile *, other_profile);

  ACE_ASSERT (op->object_key_.length () < UINT_MAX);

  return this->port_ == op->port_
    && this->object_key_ == op->object_key_
    && ACE_OS::strcmp (this->host_, op->host_) == 0
    && this->version_ == op->version_;
}

CORBA::ULong
TAO_IIOP_Profile::hash (CORBA::ULong max,
                        CORBA::Environment &env)
{
  CORBA::ULong hashval;

  env.clear ();

  // Just grab a bunch of convenient bytes and hash them; could do
  // more (hostname, full key, exponential hashing) but no real need
  // to do so except if performance requires a more costly hash.

  hashval = this->object_key_.length () * this->port_;
  hashval += this->version_.minor;

  if (this->object_key_.length () >= 4)
    {
      hashval += this->object_key_ [1];
      hashval += this->object_key_ [3];
    }

  return hashval % max;
}

int
TAO_IIOP_Profile::addr_to_string(char *buffer, size_t length)
{
  int actual_len = ACE_OS::strlen (this->host_) // chars in host name
                   + sizeof (':')               // delimiter
                   + ACE_OS::strlen ("65536")    // max port
                   + sizeof ('\0');
  if (length < actual_len)
    return -1;

  ACE_OS::sprintf (buffer, "%s:%d",
                   this->host_, port_);
  return 0;
}

const char *
TAO_IIOP_Profile::host (const char *h)
{
  if (this->host_)
    {
      delete [] this->host_;
      this->host_ = 0;
    }

  if (h)
    {
      ACE_NEW_RETURN (this->host_,
                      char[ACE_OS::strlen (h) + 1],
                      0);
      ACE_OS::strcpy (this->host_, h);
    }

  return this->host_;
}

void
TAO_IIOP_Profile::reset_hint (void)
{
  if (this->hint_)
    {
      this->hint_->cleanup_hint ();
      this->hint_ = 0;
    }
}

TAO_IIOP_Profile &
TAO_IIOP_Profile::operator= (const TAO_IIOP_Profile &src)
{
  this->version_ = src.version_;

  this->object_key_ = src.object_key_;

  this->object_addr_.set (src.object_addr_);

  this->port_ = src.port_;

  if (this->host_)
    {
      delete [] this->host_;
      this->host_ = 0;
    }

  if (src.host_)
    {
      ACE_NEW_RETURN (this->host_,
                      char[ACE_OS::strlen (src.host_) + 1],
                      *this);
      ACE_OS::strcpy (this->host_, src.host_);
    }

  return *this;
}

CORBA::String
TAO_IIOP_Profile::to_string (CORBA::Environment &env)
{
  ACE_UNUSED_ARG (env);

  CORBA::String_var key;
  TAO_POA::encode_sequence_to_string (key.inout(),
                                      this->object_key ());

  u_int buflen = (ACE_OS::strlen (::prefix_) +
                  2 /* double-slash separator */ +
                  1 /* major version */ +
                  1 /* decimal point */ +
                  1 /* minor version */ +
                  1 /* `@' character */ +
                  ACE_OS::strlen (this->host_) +
                  1 /* colon separator */ +
                  5 /* port number */ +
                  1 /* object key separator */ +
                  ACE_OS::strlen (key) +
                  1 /* zero terminator */);

  CORBA::String buf = CORBA::string_alloc (buflen);

  static const char digits [] = "0123456789";

  ACE_OS::sprintf (buf,
                   "%s//%c.%c@%s:%d%c%s",
                   ::prefix_,
                   digits [this->version_.major],
                   digits [this->version_.minor],
                   this->host_,
                   this->port_,
                   this->object_key_delimiter,
                   key.in ());
  return buf;
}

const char *
TAO_IIOP_Profile::prefix (void)
{
  return ::prefix_;
}

int
TAO_IIOP_Profile::encode (TAO_OutputCDR &stream) const
{
  // UNSIGNED LONG, protocol tag
  stream.write_ulong (this->tag ());

  // UNSIGNED LONG, number of succeeding bytes in the
  // encapsulation.  We don't actually need to make the
  // encapsulation, as nothing needs stronger alignment than
  // this longword; it guarantees the rest is aligned for us.

  CORBA::ULong hostlen = ACE_OS::strlen ((char *) this->host_);
  CORBA::ULong encap_len =
    1                              // byte order
    + 1                            // version major
    + 1                            // version minor
    + 1                            // pad byte
    + 4                            // sizeof (strlen)
    + hostlen + 1                  // strlen + null
    + (~hostlen & 01)              // optional pad byte
    + 2                            // port
    + ( hostlen & 02)              // optional pad short
    + 4                            // sizeof (key length)
    + this->object_key_.length (); // key length.
  stream.write_ulong (encap_len);

  // CHAR describing byte order, starting the encapsulation
  stream.write_octet (TAO_ENCAP_BYTE_ORDER);

  // IIOP::TAO_IOP_Version, two characters (version 1.0) padding
  stream.write_char (this->version_.major);
  stream.write_char (this->version_.minor);

  // STRING hostname from profile
  stream.write_string (this->host_);

  // UNSIGNED SHORT port number
  stream.write_ushort (this->port_);

  // OCTET SEQUENCE for object key
  stream << this->object_key_;

  return 1;
}
