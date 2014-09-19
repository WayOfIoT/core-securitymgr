/******************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#ifndef SECURITYMANAGER_H_
#define SECURITYMANAGER_H_

#include <IdentityData.h>
#include <alljoyn/Status.h>
#include <alljoyn/AllJoynStd.h>
#include <RootOfTrust.h>
#include <ApplicationInfo.h>
#include <ApplicationListener.h>
#include <StorageConfig.h>
#include <Identity.h>
#include <Storage.h>
#include <memory>

#include <qcc/String.h>

#include <qcc/Debug.h>
#define QCC_MODULE "SEC_MGR"

namespace ajn {
namespace securitymgr {
class SecurityManagerImpl;

/**
 * \class SecurityManager
 * \brief The SecurityManager enables the claiming of applications in a secure manner besides
 *        providing the needed affiliated functionalities.
 *
 * Internally it uses an ApplicationMonitor to track active applications.
 * A particular user has a SecurityManager object for each RoT he owns
 * In other words: 1 RoT = 1 SecurityManager
 */

class SecurityManager {
  private:
    /* This constructor can only be called by the factory */
    /* I don't like to pass the private key in memory like this but that is the alljoyn way of doing it (sigh) */

    SecurityManager(qcc::String userName,
                    qcc::String password,
                    IdentityData* id,                                              // Supports NULL identity
                    ajn::BusAttachment* ba,
                    const qcc::ECCPublicKey& pubKey,
                    const qcc::ECCPrivateKey& privKey,
                    const StorageConfig& storageCfg);

    QStatus GetStatus() const;

    void FlushStorage();

    SecurityManager(const SecurityManager&);
    void operator=(const SecurityManager&);

    friend class SecurityManagerFactory;

  public:
    /**
     * \brief Claim an application if it was indeed claimable.
     *        This entails installing a RoT, generating an identity certificate (based on Aboutdata)
     *        and installing that certificate.
     *
     *  QUESTION:
     *   -What information do we cache locally: it probably makes sense to store which application we claimed ?
     *      Or can we retrieve this information at run-time (privacy concerns !)
     *      OTOH: if our RoT on the application gets removed by a second RoT, can we still guarantee cache consistency..
     *   -Is this a blocking call ? if the network would go down this method call can hang for quite some time.
     *      I guess in the PoC we could keep this blocking and later have an async call with a completion callback .
     *   -Probably QStatus is not good enough to convey all the information when things go wrong security-wise. Either extend QStatus or come up with a new error.
     *
     * \param[in] app the application info of the application that we need to add the RoT to
     *
     * \retval ER_OK  on success
     * \retval others on failure
     */
    QStatus ClaimApplication(const ApplicationInfo& app);

    /**
     * \brief Install a given generated Identity on a specific application.
     *
     * \param[in] app the application info of the application that we need to add the RoT to
     *
     * \retval ER_OK  on success
     * \retval others on failure
     */
    QStatus InstallIdentity(const ApplicationInfo& app);

    /**
     * \brief Install a provided RoT on a specific application.
     *
     * \param[in] app the application info of the application that we need to add the RoT to
     * \param[in] rot the RoT that needs to be added to the application in focus
     *
     * \retval ER_OK  on success
     * \retval others on failure
     */

    QStatus AddRootOfTrust(const ApplicationInfo& app,
                           const RootOfTrust& rot);

    /**
     * \brief Remove a certain RoT from a specific application.
     *
     * \param[in] app the application info of the application that we need to have its RoT removed
     * \param[in] rot the RoT that needs to be removed from the application in focus
     *
     * \retval ER_OK  on success
     * \retval others on failure
     */
    QStatus RemoveRootOfTrust(const ApplicationInfo& app,
                              const RootOfTrust& rot);

    /**
     * \brief Get the RoT of this security manager.
     *
     * REMARK: You need this if you want to remove the RoT or export
     * the RoT (e.g. to another security manager on another physical device)
     *
     * \retval RootOfTrust the root of trust that is affiliated with this security manager.
     */
    const RootOfTrust& GetRootOfTrust() const;

    /**
     * \brief Get a list of all Applications that were discovered using About.
     *
     * \param[in] acs the claim status of the requested applications. If UNKOWN then all are returned.
     *
     * \retval vector<ApplicationInfo> on success
     * \retval empty-vector otherwise
     */
    std::vector<ApplicationInfo> GetApplications(ApplicationClaimState acs = ApplicationClaimState::UNKNOWN) const;

    /**
     * \brief Register a listener that is called-back whenever the application info is changed.
     *
     * \param[in] al ApplicationListener listener on application changes
     */
    void RegisterApplicationListener(ApplicationListener* al);

    /**
     * \brief Unregister a previously registered listener on application info changes.
     *
     * \param[in] al ApplicationListener listener on application changes
     */
    void UnregisterApplicationListener(ApplicationListener* al);

    /**
     * \brief Get the application info based on a provided busName.
     *
     * \param[in] busName the bus name associated with a specific application
     *
     * \retval ApplicationInfo* on success
     * \retval NULL on failure
     */
    ApplicationInfo* GetApplication(qcc::String& busName) const;

    ~SecurityManager();

  private:
    std::unique_ptr<SecurityManagerImpl> securityManagerImpl;
};
}
}
#undef QCC_MODULE
#endif /* SECURITYMANAGER_H_ */