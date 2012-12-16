#ifndef RSGIXS_H
#define RSGIXS_H

/*
 * libretroshare/src/gxs: gxs.h
 *
 * General Identity Exchange Service interface for RetroShare.
 *
 * Copyright 2011-2011 by Robert Fernie, Christopher Evi-Prker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */

#include "gxs/rsgxs.h"

#include <openssl/ssl.h>
#include <set>

/*!
 * GIXP: General Identity Exchange Service.
 *
 * As we're always running into troubles with GPG signatures... we are going to
 * create a layer of RSA Keys for the following properties:
 *
 * 1) RSA Keys can be Anonymous, Self-Signed with Pseudonym, Signed by GPG Key.
 * To clarify:
 *    a. This forms a layer of keys stay between GPG and pub/priv publish key ?
 *    b. Difference between anonymous and pseudonym keys?
 *            - Anonymous cannot be signed?
 *            -
 *    c. To some extent this determines security model of RsGeneralExchangeService

 *	- Anonymous & Pseudonym Keys will be shared network-wide (Hop by Hop).
        - GPG signed Keys will only be shared if we can validate the signature
                (providing similar behaviour to existing GPG Keys).
        - GPG signed Keys can optionally be marked for Network-wide sharing.
 * 2) These keys can be used anywhere, specifically in the protocols described below.
 * 3) These keys can be used to sign, encrypt, verify & decrypt
 * 4) Keys will never need to be directly accessed - stored in this class.
 *     a. I guess can work solely through Id
 *     b. Use Case: Receivve a message, has a key id, request
 * 5) They will be cached locally and exchanged p2p, by pull request.
 * 6) This class will use the generalised packet storage for efficient caching & loading.
 * 7) Data will be stored encrypted.
 */


/*!
 * Storage class for private and public publish keys
 *
 */
class GixsKey
{
        KeyRef mKeyId;

        /// public key
        EVP_PKEY *mPubKey;

        ///  NULL if non-existant */
        EVP_PKEY *mPrivKey;
};

/*!
 *
 *
 */
class KeyRef {

    std::string refId;

};


class KeyRefSet {
    std::set<KeyRef> mKeyRefSet;
};

class SignatureSet {
    std::set<RsGxsSignature> mSignatureSet;
};

/*!
 *
 *
 */
class RsGxsSignature {

    KeyRef mKeyRef;
};

/*!
 * This is the actual identity \n
 * In a sense the group description with the GixsKey the "message"
 */
class RsGixsProfile {

public:

    KeyRef mKeyRef;
    std::string name;

    /// may be superseded by newer timestamps
    time_t mTimeStamp;
    uint32_t mProfileType;

    // TODO: add permissions members

    RsGxsSignature mSignature;

};

/*!
 * Retroshare general identity exchange service
 *
 * Purpose: \n
 *   Provides a means to distribute identities among peers \n
 *   Also provides encyption, decryption, verification, \n
 *   and signing functionality using any created or received identities \n
 *
 * This may best be implemented as a singleton like current AuthGPG? \n
 *
 */
class RsIdentityExchangeService : RsGxsService
{
public:

    enum IdentityType { Pseudonym, Signed, Anonymous };

    RsGixs();

    /*!
     * creates gixs profile and shares it
     * @param profile
     * @param type the type of profile to create, self signed, anonymous, and GPG signed
     */
    virtual bool createKey(RsGixsProfile& profile, uint32_t type) = 0; /* fills in mKeyId, and signature */

    /*!
     * Use to query a whether given key is available by its key reference
     * @param keyref the keyref of key that is being checked for
     * @return true if available, false otherwise
     */
    virtual bool haveKey(const KeyRef& keyref) = 0;

    /*!
     * Use to query whether private key member of the given key reference is available
     * @param keyref the KeyRef of the key being checked for
     * @return true if private key is held here, false otherwise
     */
    virtual bool havePrivateKey(const KeyRef& keyref) = 0;

    /*!
     * Use to request a given key reference
     * @param keyref the KeyRef of the key being requested
     * @return will
     */
    virtual bool requestKey(const KeyRef& keyref) = 0;

    /*!
     * Retrieves a key identity
     * @param keyref
     * @return a pointer to a valid profile if successful, otherwise NULL
     *
     */
    virtual RsGixsProfile* getProfile(const KeyRef& keyref) = 0;


    /*** process data ***/

    /*!
     * Use to sign data with a given key
     * @param keyref the key to sign the data with
     * @param data the data to be signed
     * @param dataLen the length of the data
     * @param signature is set with the signature from signing with keyref
     * @return false if signing failed, true otherwise
     */
    virtual bool sign(const KeyRef& keyref, unsigned char* data, uint32_t dataLen, std::string& signature) = 0;

    /*!
     * Verify that the data is signed by the key owner
     * @param keyref
     * @param data
     * @param dataLen
     * @param signature
     * @return false if verification failed, false otherwise
     */
    virtual bool verify(const KeyRef& keyref, unsigned char* data, int dataLen, std::string& signature) = 0;

    /*!
     * Attempt to decrypt data with a given key
     * @param keyref
     * @param data data to be decrypted
     * @param dataLen length of data
     * @param decryptedData decrypted data
     * @param decryptDataLen length of decrypted data
     * @return false
     */
    virtual bool decrypt(const KeyRef& keyref, unsigned char* data, int dataLen,
                 unsigned char*& decryptedData, uint32_t& decyptDataLen) = 0;

    /*!
     * Attempt to encrypt data with a given key
     * @param keyref
     * @param data data to be encrypted
     * @param dataLen length of data
     * @param encryptedData encrypted data
     * @param encryptDataLen length of encrypted data
     */
    virtual bool encrypt(const KeyRef& keyref, unsigned char* data, int dataLen,
                 unsigned char*& encryptedData, uint32_t& encryptDataLen) = 0;

};


#endif // RSGIXS_H
