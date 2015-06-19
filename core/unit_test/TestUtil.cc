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
#include <qcc/Util.h>
#include <qcc/Environ.h>
#include <stdlib.h>
#include <qcc/Thread.h>

using namespace ajn::securitymgr;
using namespace std;

#define STORAGE_DEFAULT_PATH "/tmp/secmgr.db"
#define STORAGE_DEFAULT_PATH_KEY "STORAGE_PATH"

namespace secmgrcoretest_unit_testutil {
TestApplicationListener::TestApplicationListener(qcc::Condition& _sem,
                                                 qcc::Mutex& _lock) :
    sem(_sem), lock(_lock)
{
}

void TestApplicationListener::OnApplicationStateChange(const OnlineApplication* old,
                                                       const OnlineApplication* updated)
{
    const OnlineApplication* info = updated ? updated : old;
    ApplicationListener::PrintStateChangeEvent(old, updated);
    lock.Lock();
    events.push_back(*info);
    sem.Broadcast();
    lock.Unlock();
}

BasicTest::BasicTest() : storage(nullptr), ca(nullptr)
{
    secMgr = NULL;
    tal = NULL;
    ba = NULL;
    stub = NULL;
}

void BasicTest::SetUp()
{
    qcc::String storage_path;

    storage_path = qcc::Environ::GetAppEnviron()->Find(STORAGE_DEFAULT_PATH_KEY, STORAGE_DEFAULT_PATH);
    qcc::Environ::GetAppEnviron()->Add(STORAGE_DEFAULT_PATH_KEY, STORAGE_DEFAULT_PATH);

    remove(storage_path.c_str());
    // clean up any lingering stub keystore
    qcc::String fname = GetHomeDir();
    fname.append("/.alljoyn_keystore/stub.ks");
    remove(fname.c_str());

    SecurityAgentFactory& secFac = SecurityAgentFactory::GetInstance();
    SQLStorageFactory& storageFac = SQLStorageFactory::GetInstance();

    ba = new BusAttachment("test", true);
    ASSERT_TRUE(ba != NULL);
    ASSERT_EQ(ER_OK, ba->Start());
    ASSERT_EQ(ER_OK, ba->Connect());

    ba->RegisterAboutListener(testAboutListener);

    /* Passing NULL into WhoImplements will listen for all About announcements */

    if (ER_OK != ba->WhoImplements(NULL)) {
        printf("WhoImplements NULL failed.\n");
    }

    ASSERT_EQ(ER_OK, storageFac.GetStorages("test", ca, storage));
    SecurityAgentIdentityInfo securityAgentIdendtityInfo;
    securityAgentIdendtityInfo.name = "UnitTestAgent";
    securityAgentIdendtityInfo.version = "1.0";
    securityAgentIdendtityInfo.vendor = "Nameless Vendor";
    secMgr = secFac.GetSecurityAgent(securityAgentIdendtityInfo, ca, ba);
    ASSERT_TRUE(secMgr != NULL);

    secMgr->SetManifestListener(&aa);

    tal = new TestApplicationListener(sem, lock);
    secMgr->RegisterApplicationListener(tal);
}

void BasicTest::UpdateLastAppInfo()
{
    lock.Lock();
    if (tal->events.size()) {
        vector<OnlineApplication>::iterator it = tal->events.begin();
        lastAppInfo = *it;
        tal->events.erase(it);
    }
    lock.Unlock();
}

bool BasicTest::WaitForState(ajn::PermissionConfigurator::ApplicationState newState,
                             ajn::securitymgr::ApplicationRunningState newRunningState, const int updatesPending)
{
    lock.Lock();
    printf("\nWaitForState: waiting for event(s) ...\n");
    //Prior to entering this function, the test should have taken an action which leads to one or more events.
    //These events are handled in a separate thread.
    do {
        if (tal->events.size()) { //if event is in the queue, we will return immediately
            UpdateLastAppInfo(); //update latest value.
            printf("WaitForState: Checking event ... ");
            if (lastAppInfo.claimState == newState && MatchesRunningState(lastAppInfo, newRunningState) &&
                ((updatesPending == -1 ? 1 : ((bool)updatesPending == lastAppInfo.updatesPending)))) {
                printf("ok\n");
                lock.Unlock();
                return true;
            }
            printf("not ok, waiting/checking for next event\n");
        } else {
            QStatus status = sem.TimedWait(lock, 10000);
            if (ER_OK != status) {
                printf("timeout- failing test - %i\n", status);
                break;
            }
            assert(tal->events.size()); // assume TimedWait returns != ER_OK in case of timeout
        }
    } while (true);
    printf("WaitForState failed.\n");
    printf("\tClaimableState: expected = %s, got %s\n", ToString(newState),
           ToString(lastAppInfo.claimState));
    printf("\tRunningState: expected = %s\n", ToString(
               newRunningState));
    if (updatesPending != -1) {
        printf("\tUpdatesPending : expected = %s, got %s\n", (updatesPending ? "True" : "False"),
               (lastAppInfo.updatesPending ? "True" : "False"));
    }

    lock.Unlock();
    return false;
}

bool BasicTest::MatchesRunningState(const OnlineApplication& app,
                                    ajn::securitymgr::ApplicationRunningState runningState)
{
    if (runningState == ajn::securitymgr::ApplicationRunningState::STATE_RUNNING && !app.busName.empty()) {
        return true;
    }

    if (runningState == ajn::securitymgr::ApplicationRunningState::STATE_NOT_RUNNING  && app.busName.empty()) {
        return true;
    }
    return false;
}

void BasicTest::TearDown()
{
    if (tal) {
        secMgr->UnregisterApplicationListener(tal);
        delete tal;
        tal = NULL;
    }
    delete secMgr;
    secMgr = NULL;

    ba->UnregisterAboutListener(testAboutListener);

    delete ba;
    ba = NULL;
    delete stub;
    stub = NULL;
    storage->Reset();
}
}
