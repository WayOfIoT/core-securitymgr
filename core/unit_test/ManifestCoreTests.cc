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

class ManifestCoreTests :
    public BasicTest {
  private:

  protected:

  public:
    ManifestCoreTests()
    {
    }

    QStatus GenerateManifest(PermissionPolicy::Rule** retRules,
                             size_t* count)
    {
        *count = 2;
        PermissionPolicy::Rule* rules = new PermissionPolicy::Rule[*count];
        rules[0].SetInterfaceName("org.allseenalliance.control.TV");
        PermissionPolicy::Rule::Member* prms = new PermissionPolicy::Rule::Member[2];
        prms[0].SetMemberName("Up");
        prms[0].SetMemberType(PermissionPolicy::Rule::Member::METHOD_CALL);
        prms[0].SetActionMask(PermissionPolicy::Rule::Member::ACTION_MODIFY);
        prms[1].SetMemberName("Down");
        prms[1].SetMemberType(PermissionPolicy::Rule::Member::METHOD_CALL);
        prms[1].SetActionMask(PermissionPolicy::Rule::Member::ACTION_MODIFY);
        rules[0].SetMembers(2, prms);

        rules[1].SetInterfaceName("org.allseenalliance.control.Mouse*");
        prms = new PermissionPolicy::Rule::Member[1];
        prms[0].SetMemberName("*");
        prms[0].SetActionMask(PermissionPolicy::Rule::Member::ACTION_MODIFY);
        rules[1].SetMembers(1, prms);

        *retRules = rules;
        return ER_OK;
    }
};

TEST_F(ManifestCoreTests, SuccessfulGetManifest) {
    bool claimAnswer = true;
    TestClaimListener tcl(claimAnswer);

    /* Start the stub */
    stub = new Stub(&tcl);

    /* Wait for signals */
    ASSERT_TRUE(WaitForState(ajn::PermissionConfigurator::CLAIMABLE, ajn::securitymgr::STATE_RUNNING));

    IdentityInfo idInfo;
    idInfo.guid = GUID128("abcdef123456789");
    idInfo.name = "TestIdentity";
    ASSERT_EQ(storage->StoreIdentity(idInfo), ER_OK);

    /* Set manifest */
    PermissionPolicy::Rule* rules;
    size_t count;
    GenerateManifest(&rules, &count);
    stub->SetUsedManifest(rules, count);

    /* Claim! */
    ASSERT_EQ(ER_OK, secMgr->Claim(lastAppInfo, idInfo));

    /* Check security signal */
    ASSERT_TRUE(WaitForState(ajn::PermissionConfigurator::CLAIMED, ajn::securitymgr::STATE_RUNNING));

    /* Retrieve manifest */
    PermissionPolicy::Rule* retrievedRules = NULL;
    size_t retrievedCount = 0;
    Manifest mf;
    ASSERT_EQ(ER_OK, storage->GetManifest(lastAppInfo, mf));
    ASSERT_EQ(ER_OK, mf.GetRules(&retrievedRules, &retrievedCount));

    /* Compare set and retrieved manifest */
    ASSERT_EQ(count, retrievedCount);
    for (int i = 0; i < static_cast<int>(count); i++) {
        ASSERT_EQ(rules[i], retrievedRules[i]);
    }

    /* Clear the keystore of the stub */
    stub->Reset();

    /* Stop the stub */
    delete stub;
    stub = NULL;
    ASSERT_TRUE(WaitForState(ajn::PermissionConfigurator::CLAIMED, ajn::securitymgr::STATE_NOT_RUNNING));
}
} // namespace
