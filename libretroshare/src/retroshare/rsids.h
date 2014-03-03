// This class aims at defining a generic ID type that is a list of bytes. It
// can be converted into a hexadecial string for printing, mainly) or for
// compatibility with old methods.
//
// To use this class, derive your own ID type from it. Examples include:
//
// 	class RsPgpId: public t_RsGenericIdType<8> 
// 	{
// 		[..]
// 	};
//
// 	class PGPFingerprintType: public t_RsGenericIdType<20> 
// 	{
// 		[..]
// 	};
//
// With this, there is no implicit conversion between subtypes, and therefore ID mixup 
// is impossible.
//
// A simpler way to make ID types is to 
// 	typedef t_RsGenericIdType<MySize> MyType ;
//
// ID Types with different lengths will be incompatible on compilation.
//
// Warning: never store references to a t_RsGenericIdType accross threads, since the 
// 			cached string convertion is not thread safe.
//

#pragma once

#include <stdexcept>
#include <string>
#include <iostream>
#include <ostream>
#include <string.h>
#include <stdint.h>
#include <util/rsrandom.h>

template<uint32_t ID_SIZE_IN_BYTES,bool UPPER_CASE,uint32_t UNIQUE_IDENTIFIER> class t_RsGenericIdType
{
	public:
		t_RsGenericIdType() 
		{ 
			memset(bytes,0,ID_SIZE_IN_BYTES) ; 	// by default, ids are set to null()
		}
		virtual ~t_RsGenericIdType() {}

		// Explicit constructor from a hexadecimal string
		//
		explicit t_RsGenericIdType(const std::string& hex_string) ;

		// Explicit constructor from a byte array. The array should have size at least ID_SIZE_IN_BYTES
		//
		explicit t_RsGenericIdType(const unsigned char bytes[]) ;

		// Random initialization. Can be useful for testing.
		//
		static t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER> random() 
		{
			t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER> id ;

			for(uint32_t i=0;i<ID_SIZE_IN_BYTES;++i)
				id.bytes[i] = RSRandom::random_u32() & 0xff ;

			return id ;
		}

		inline void operator=(const std::string& str)
		{
			t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER> temp = t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER>(str);

            for(uint32_t i = 0; i < ID_SIZE_IN_BYTES; i++)
				this->bytes[i] = temp.toByteArray()[i];
		}

		inline void clear() { bzero(bytes,SIZE_IN_BYTES) ; }

		// Converts to a std::string using cached value. 
		//
		const unsigned char *toByteArray() const { return &bytes[0] ; }
		static const uint32_t SIZE_IN_BYTES = ID_SIZE_IN_BYTES ;

		inline bool operator==(const t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER>& fp) const { return !memcmp(bytes,fp.bytes,ID_SIZE_IN_BYTES) ; }
		inline bool operator!=(const t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER>& fp) const { return !!memcmp(bytes,fp.bytes,ID_SIZE_IN_BYTES); }
		inline bool operator< (const t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER>& fp) const { return (memcmp(bytes,fp.bytes,ID_SIZE_IN_BYTES) < 0) ; }

		inline bool isNull() const 
		{ 
			for(uint32_t i=0;i<SIZE_IN_BYTES;++i) 
				if(bytes[i] != 0)
					return false ;
			return true ;
		} 

		friend std::ostream& operator<<(std::ostream& out,const t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER>& id)
		{
			return out << id.toStdString(UPPER_CASE) ;
		}

		inline std::string toStdString() const { return toStdString(UPPER_CASE) ; }

		inline uint32_t serial_size() const { return SIZE_IN_BYTES ; }
        bool serialise(void *data,uint32_t pktsize,uint32_t& offset) const
		{
			if(offset + SIZE_IN_BYTES > pktsize)
				return false ;

			memcpy(&((uint8_t*)data)[offset],bytes,SIZE_IN_BYTES) ;
			offset += SIZE_IN_BYTES ;
			return true ;
		}
		bool deserialise(void *data,uint32_t pktsize,uint32_t& offset)
		{
			if(offset + SIZE_IN_BYTES > pktsize)
				return false ;

			memcpy(bytes,&((uint8_t*)data)[offset],SIZE_IN_BYTES) ;
			offset += SIZE_IN_BYTES ;
			return true ;
		}
	private:
		std::string toStdString(bool upper_case) const ;

		unsigned char bytes[ID_SIZE_IN_BYTES] ;
};

template<uint32_t ID_SIZE_IN_BYTES,bool UPPER_CASE,uint32_t UNIQUE_IDENTIFIER> std::string t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER>::toStdString(bool upper_case) const
{
	static const char outh[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' } ;
	static const char outl[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' } ;

	std::string res(ID_SIZE_IN_BYTES*2,' ') ;

	for(uint32_t j = 0; j < ID_SIZE_IN_BYTES; j++)
		if(upper_case)
		{
			res[2*j  ] = outh[ (bytes[j]>>4) ] ;
			res[2*j+1] = outh[ bytes[j] & 0xf ] ;
		}
		else
		{
			res[2*j  ] = outl[ (bytes[j]>>4) ] ;
			res[2*j+1] = outl[ bytes[j] & 0xf ] ;
		}

	return res ;
}
void chris_test();
template<uint32_t ID_SIZE_IN_BYTES,bool UPPER_CASE,uint32_t UNIQUE_IDENTIFIER> t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER>::t_RsGenericIdType(const std::string& s) 
{
	try
	{
		int n=0;
		if(s.length() != ID_SIZE_IN_BYTES*2)
		{
			throw std::runtime_error("t_RsGenericIdType<>::t_RsGenericIdType(std::string&): supplied string in constructor has wrong size.") ;
			chris_test();
		}

		for(uint32_t i = 0; i < ID_SIZE_IN_BYTES; ++i)
		{
			bytes[i] = 0 ;

			for(int k=0;k<2;++k)
			{
				char b = s[n++] ;

				if(b >= 'A' && b <= 'F')
					bytes[i] += (b-'A'+10) << 4*(1-k) ;
				else if(b >= 'a' && b <= 'f')
					bytes[i] += (b-'a'+10) << 4*(1-k) ;
				else if(b >= '0' && b <= '9')
					bytes[i] += (b-'0') << 4*(1-k) ;
				else
					throw std::runtime_error("t_RsGenericIdType<>::t_RsGenericIdType(std::string&): supplied string is not purely hexadecimal") ;
			}
		}
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		chris_test();
		clear() ;
	}
}

template<uint32_t ID_SIZE_IN_BYTES,bool UPPER_CASE,uint32_t UNIQUE_IDENTIFIER> t_RsGenericIdType<ID_SIZE_IN_BYTES,UPPER_CASE,UNIQUE_IDENTIFIER>::t_RsGenericIdType(const unsigned char *mem) 
{
	if(mem == NULL)
		memset(bytes,0,ID_SIZE_IN_BYTES) ;
	else
		memcpy(bytes,mem,ID_SIZE_IN_BYTES) ;
}

static const int SSL_ID_SIZE              = 16 ;	// = CERTSIGNLEN
static const int CERT_SIGN_LEN            = 16 ;	// = CERTSIGNLEN
static const int PGP_KEY_ID_SIZE          =  8 ;
static const int PGP_KEY_FINGERPRINT_SIZE = 20 ;
static const int SHA1_SIZE                = 20 ;

// These constants are random, but should be different, in order to make the various IDs incompatible with each other.
//
static const uint32_t RS_GENERIC_ID_SSL_ID_TYPE          = 0x0001 ;
static const uint32_t RS_GENERIC_ID_PGP_ID_TYPE          = 0x0002 ;
static const uint32_t RS_GENERIC_ID_SHA1_ID_TYPE         = 0x0003 ;
static const uint32_t RS_GENERIC_ID_PGP_FINGERPRINT_TYPE = 0x0004 ;
static const uint32_t RS_GENERIC_ID_GXS_GROUP_ID_TYPE    = 0x0005 ;
static const uint32_t RS_GENERIC_ID_GXS_ID_TYPE          = 0x0006 ;
static const uint32_t RS_GENERIC_ID_GXS_MSG_ID_TYPE      = 0x0007 ;
static const uint32_t RS_GENERIC_ID_GXS_CIRCLE_ID_TYPE   = 0x0008 ;

typedef t_RsGenericIdType<  SSL_ID_SIZE             , false, RS_GENERIC_ID_SSL_ID_TYPE>          SSLIdType ;
typedef t_RsGenericIdType<  PGP_KEY_ID_SIZE         , true,  RS_GENERIC_ID_PGP_ID_TYPE>          PGPIdType ;
typedef t_RsGenericIdType<  SHA1_SIZE               , false, RS_GENERIC_ID_SHA1_ID_TYPE>         Sha1CheckSum ;
typedef t_RsGenericIdType<  PGP_KEY_FINGERPRINT_SIZE, true,  RS_GENERIC_ID_PGP_FINGERPRINT_TYPE> PGPFingerprintType ;

typedef t_RsGenericIdType<  CERT_SIGN_LEN           , false, RS_GENERIC_ID_GXS_GROUP_ID_TYPE   > GXSGroupId ;
typedef t_RsGenericIdType<  CERT_SIGN_LEN           , false, RS_GENERIC_ID_GXS_ID_TYPE         > GXSId ;
typedef t_RsGenericIdType<  CERT_SIGN_LEN           , false, RS_GENERIC_ID_GXS_CIRCLE_ID_TYPE  > GXSCircleId ;

