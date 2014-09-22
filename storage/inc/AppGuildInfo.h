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

#ifndef APPGUILDINFO_H_
#define APPGUILDINFO_H_

#include <qcc/String.h>
#include <PublicKey.h>

namespace ajn {
namespace securitymgr {
/*
 * \brief Represents any application info that is persisted in local storage.
 */
struct ManagedApplicationInfo {
    PublicKey publicKey;
    qcc::String userDefinedName;
    qcc::String deviceName;
    qcc::String appName;
    qcc::String appID;
    qcc::String manifest;
};

struct GuildInfo {
    qcc::String guid;
    qcc::String name;
    qcc::String desc;

    bool operator==(const GuildInfo& gi) const
    {
        if (guid != gi.guid) {
            return false;
        }

        if (name != gi.name) {
            return false;
        }

        if (desc != gi.desc) {
            return false;
        }
        return true;
    }
};
}
}
#endif /* APPGUILDINFO_H_ */