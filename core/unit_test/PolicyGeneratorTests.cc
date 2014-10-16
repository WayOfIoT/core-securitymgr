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

#include "gtest/gtest.h"

#include <stdio.h>
#include <PolicyGenerator.h>

using namespace std;
using namespace ajn;
using namespace qcc;
using namespace securitymgr;

TEST(PolicyGeneratorTest, BasicTest) {
    PermissionPolicy pol;
    GUID128 guildID;
    qcc::String guildIDString = BytesToHexString(guildID.GetBytes(), GUID128::SIZE);

    vector<GUID128> guilds;
    guilds.push_back(guildID);

    PolicyGenerator::DefaultPolicy(guilds, pol);
    qcc::String policyString = pol.ToString();
    ASSERT_EQ((size_t)1, pol.GetTermsSize());
    ASSERT_NE(string::npos, policyString.find(guildIDString));

    GUID128 guildID2;
    guilds.push_back(guildID2);
    qcc::String guildID2String = BytesToHexString(guildID2.GetBytes(), GUID128::SIZE);

    PolicyGenerator::DefaultPolicy(guilds, pol);
    policyString = pol.ToString();

    ASSERT_EQ((size_t)2, pol.GetTermsSize());
    ASSERT_NE(string::npos, policyString.find(guildIDString));
    ASSERT_NE(string::npos, policyString.find(guildID2String));
}