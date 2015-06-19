/******************************************************************************
 * Copyright (c) AllSeen Alliance. All rights reserved.
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

#include "TestUtil.h"

namespace secmgrcoretest_unit_nominaltests {
using namespace secmgrcoretest_unit_testutil;

using namespace ajn::securitymgr;

class IdentityCoreTests :
    public BasicTest {
  private:

  protected:

  public:
    IdentityCoreTests()
    {
    }
};
TEST_F(IdentityCoreTests, SuccessfulInstallIdentity) {
    bool claimAnswer = true;
    TestClaimListener tcl(claimAnswer);

    /* Start the stub */
    stub = new Stub(&tcl);

    /* Wait for signals */
    ASSERT_TRUE(WaitForState(ajn::PermissionConfigurator::CLAIMABLE, ajn::securitymgr::STATE_RUNNING));

    IdentityInfo info;
    info.name = "MyName";
    ASSERT_EQ(ER_OK, storage->StoreIdentity(info));

    /* Claim! */
    ASSERT_EQ(ER_OK, secMgr->Claim(lastAppInfo, info));
    ASSERT_TRUE(WaitForState(ajn::PermissionConfigurator::CLAIMED, ajn::securitymgr::STATE_RUNNING));

    /* Try to install identity again */
    ASSERT_EQ(ER_OK, storage->UpdateIdentity(lastAppInfo, info));
    ASSERT_TRUE(WaitForState(ajn::PermissionConfigurator::CLAIMED, ajn::securitymgr::STATE_RUNNING, true));
    ASSERT_TRUE(WaitForState(ajn::PermissionConfigurator::CLAIMED, ajn::securitymgr::STATE_RUNNING, false));

    OnlineApplication checkUpdatesPendingInfo;
    checkUpdatesPendingInfo.publicKey = lastAppInfo.publicKey;
    ASSERT_EQ(ER_OK, secMgr->GetApplication(checkUpdatesPendingInfo));
    ASSERT_FALSE(checkUpdatesPendingInfo.updatesPending);

    /* Clear the keystore of the stub */
    stub->Reset();

    /* Stop the stub */
    delete stub;
    stub = NULL;
    ASSERT_TRUE(WaitForState(ajn::PermissionConfigurator::CLAIMED, ajn::securitymgr::STATE_NOT_RUNNING));
}
} // namespace
