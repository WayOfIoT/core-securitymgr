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

#include <alljoyn/securitymgr/ApplicationListener.h>
#include <alljoyn/securitymgr/ApplicationState.h>

#include <iostream>

using namespace std;

namespace ajn {
namespace securitymgr {
void ApplicationListener::PrintStateChangeEvent(const OnlineApplication* old,
                                                const OnlineApplication* updated)
{
    const OnlineApplication* info = updated ? updated : old;
    cout << "  Application updated:" << endl;
    cout << "  ====================" << endl;
    cout << "  Busname           : " << info->busName << endl;
    cout << "  Claim state       : "
         << (old ? ToString(old->claimState) : "UNKNOWN") << " --> "
         << (updated ? ToString(updated->claimState) : "UNKNOWN") << endl;
    cout << "  Updates pending   : " << (info->updatesPending ? "true" : "false");
    cout << endl << "> " << flush;
}
}
}
