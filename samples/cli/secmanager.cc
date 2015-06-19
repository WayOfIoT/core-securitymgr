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

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <map>
#include <alljoyn/securitymgr/Application.h>
#include <alljoyn/securitymgr/SecurityAgent.h>
#include <alljoyn/securitymgr/sqlstorage/SQLStorageFactory.h>
#include <alljoyn/securitymgr/SecurityAgentFactory.h>
#include <alljoyn/securitymgr/ApplicationListener.h>
#include <alljoyn/securitymgr/SyncError.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/Init.h>
#include <alljoyn/AboutListener.h>
#include <alljoyn/AboutObj.h>
#include <string>
#include <sstream>
#include <alljoyn/securitymgr/PolicyGenerator.h>
#include <alljoyn/PermissionPolicy.h>
#include <qcc/Mutex.h>
#include <memory>

using namespace std;
using namespace qcc;
using namespace ajn;
using namespace ajn::securitymgr;

#define GROUPINFO_DELIMITER "/"
#define GROUP_DESC_MAX 200
#define GROUP_ID_MAX 32

static std::map<qcc::String, qcc::ECCPublicKey> keys;
static qcc::Mutex lock;
static std::map<qcc::String, ApplicationMetaData> aboutCache; // Key is busname
static qcc::Mutex aboutCachelock;

static qcc::String toKeyID(const qcc::ECCPublicKey& key)
{
    qcc::GUID128 guid;
    guid.SetBytes(key.x);
    return guid.ToString();
}

static qcc::String addKeyID(const qcc::ECCPublicKey& key)
{
    qcc::String id = toKeyID(key);
    lock.Lock(__FILE__, __LINE__);
    std::map<qcc::String, qcc::ECCPublicKey>::iterator it = keys.find(id);

    if (it == keys.end()) {
        keys[id] = key;
    }
    lock.Unlock(__FILE__, __LINE__);
    return id;
}

static bool getKey(string appId, qcc::ECCPublicKey& key)
{
    qcc::String id(appId.c_str());
    lock.Lock(__FILE__, __LINE__);
    std::map<qcc::String, qcc::ECCPublicKey>::iterator it = keys.find(id);
    bool found = false;
    if (it != keys.end()) {
        key = it->second;
        found = true;
    }
    lock.Unlock(__FILE__, __LINE__);
    return found;
}

const char* ToString(SyncErrorType errorType)
{
    switch (errorType) {
    case SYNC_ER_UNKNOWN:
        return "SYNC_ER_UNKNOWN";

    case SYNC_ER_RESET:
        return "SYNC_ER_RESET";

    case SYNC_ER_IDENTITY:
        return "SYNC_ER_IDENTITY";

    case SYNC_ER_MEMBERSHIP:
        return "SYNC_ER_MEMBERSHIP";

    case SYNC_ER_POLICY:
        return "SYNC_ER_POLICY";

    default:
        return "SYNC_ER_UNEXPECTED";
    }
};

// Event listener for the monitor
class EventListener :
    public ajn::securitymgr::ApplicationListener {
    virtual void OnApplicationStateChange(const OnlineApplication* old,
                                          const OnlineApplication* updated)
    {
        const OnlineApplication* app = old == NULL ? updated : old;
        ApplicationListener::PrintStateChangeEvent(old, updated);
        cout << "  Application Id: " << addKeyID(app->publicKey) << endl << endl;
    }

    virtual void OnSyncError(const SyncError* er)
    {
        cout << "  Synchronization error" << endl;
        cout << "  =====================" << endl;
        cout << "  Application bus name  : " << er->app.busName.c_str() << endl;
        cout << "  Type              : " << ToString(er->type) << endl;
        cout << "  Remote status     : " << QCC_StatusText(er->status) << endl;
        switch (er->type) {
        case SYNC_ER_IDENTITY:
            cout << "  IdentityCert SN   : " << er->GetIdentityCertificate()->GetSerial() << endl;
            break;

        case SYNC_ER_MEMBERSHIP:
            cout << "  MembershipCert SN :  " << er->GetMembershipCertificate()->GetSerial() << endl;
            break;

        case SYNC_ER_POLICY:
            cout << "  Policy version    : " << er->GetPolicy()->GetVersion() << endl;
            break;

        default:
            break;
        }
    }
};

class CliAboutListner :
    public AboutListener {
    void Announced(const char* busName, uint16_t version,
                   SessionPort port, const MsgArg& objectDescriptionArg,
                   const MsgArg& aboutDataArg)
    {
        QCC_UNUSED(objectDescriptionArg);
        QCC_UNUSED(port);
        QCC_UNUSED(version);

        AboutData aboutData(aboutDataArg);
        char* appName;
        aboutData.GetAppName(&appName);
        char* deviceName;
        aboutData.GetDeviceName(&deviceName);

        cout << "\nReceived About signal:";
        cout << "\n BusName          : " << busName;
        cout << "\n Application Name : " << appName;
        cout << "\n Device Name      : " << deviceName << endl << endl;

        ApplicationMetaData appMetaData;
        appMetaData.deviceName = deviceName;
        appMetaData.appName = appName;

        aboutCachelock.Lock(__FILE__, __LINE__);
        aboutCache[busName] = appMetaData;
        aboutCachelock.Unlock(__FILE__, __LINE__);
    }
};

std::ostream& operator<<(std::ostream& strm, const GroupInfo& gi)
{
    return strm << "Group: (" << gi.guid.ToString() << " / " << gi.name << " / " << gi.desc
                << ")";
}

static vector<string> split(const string& s, char delim)
{
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

static void list_claimable_applications(SecurityAgent* secAgent)
{
    vector<OnlineApplication> claimableApps;
    secAgent->GetApplications(claimableApps);

    if (0 == claimableApps.size()) {
        cout << "There are currently no claimable applications published"
             << endl;
        return;
    } else {
        cout << "There are currently " << claimableApps.size()
             << " unclaimed applications published" << endl;
    }

    vector<OnlineApplication>::const_iterator it = claimableApps.begin();
    for (int i = 0; it < claimableApps.end(); ++it) {
        const OnlineApplication& info = *it;
        cout << i << ". id: " << toKeyID(info.publicKey) << " -  bus name: " << info.busName << " - claim state: " <<
            ToString(info.claimState) << endl;
    }
}

static void list_claimed_applications(const shared_ptr<UIStorage>& uiStorage)
{
    vector<Application> applications;
    uiStorage->GetManagedApplications(applications);

    if (!applications.empty()) {
        cout << "  Following claimed applications have been found:" << endl;
        cout << "  ===============================================" << endl;

        vector<ajn::securitymgr::Application>::const_iterator it =
            applications.begin();
        for (int i = 0; it < applications.end(); ++it, i++) {
            const ajn::securitymgr::Application& info = *it;
            cout << i << ". id: " << toKeyID(info.publicKey) << endl;
        }
    } else {
        cout << "There are currently no claimed applications" << endl;
    }
}

class CLIManifestListener :
    public ManifestListener {
  public:

    bool ApproveManifest(const OnlineApplication& app,
                         const Manifest& manifest)
    {
        QCC_UNUSED(app);

        PermissionPolicy::Rule* manifestRules;
        size_t manifestRulesCount;
        manifest.GetRules(&manifestRules, &manifestRulesCount);

        bool result = false;

        cout << "The application requests the following rights:" << endl;
        for (size_t i = 0; i < manifestRulesCount; i++) {
            cout << manifestRules[i].ToString().c_str();
        }
        cout << "Accept (y/n)? ";

        string input;
        getline(cin, input);

        char cmd;
        cmd = input[0];

        switch (cmd) {
        case 'y':
        case 'Y':
            result = true;
            break;
        }

        return result;
    }
};

static void claim_application(SecurityAgent* secAgent,
                              const shared_ptr<UIStorage>& uiStorage,
                              const string& arg)
{
    if (arg.empty()) {
        cout << "Please provide an application ID" << endl;
        return;
    }

    OnlineApplication app;

    if (!getKey(arg, app.publicKey) || ER_END_OF_DATA == secAgent->GetApplication(
            app)) {
        cout
            << "Invalid Application ..."
            << endl;
        return;
    } else {
        vector<IdentityInfo> list;
        uiStorage->GetIdentities(list);
        if (list.size() == 0) {
            cout
                << "No identity defined..."
                << endl;
            return;
        }
        if (ER_OK != secAgent->Claim(app, list.at(0))) {
            cout
                << "Failed to claim application..."
                << endl;
            return;
        }
    }
}

static void unclaim_application(const shared_ptr<UIStorage>& uiStorage,
                                const string& arg)
{
    if (arg.empty()) {
        cout << "Please provide an Application ID..." << endl;
        return;
    }

    ajn::securitymgr::Application app;

    if (!getKey(arg, app.publicKey) || ER_END_OF_DATA == uiStorage->GetManagedApplication(app)) {
        cout << "Could not find application" << endl;
        return;
    }

    if (ER_OK != uiStorage->RemoveApplication(app)) {
        cout << "Failed to unclaim application" << endl;
        return;
    }
}

static void set_app_meta_data_and_name(const shared_ptr<UIStorage>& uiStorage, SecurityAgent* secAgent,
                                       const string& arg)
{
    vector<string> args = split(arg, ' ');

    if (args.size() != 2) {
        cerr << "Please provide an application id and a user defined name." << endl;
        return;
    }

    OnlineApplication app;

    if (!getKey(args[0], app.publicKey) || ER_OK != uiStorage->GetManagedApplication(app)) {
        cerr << "Could not find application." << endl;
        return;
    }

    ApplicationMetaData appMetaData;
    appMetaData.userDefinedName = args[1].c_str();

    if (ER_OK != secAgent->GetApplication(app)) { // Fetches the online status which includes the busname.
        cout << "Could not find online application status..." << endl;
    } else {
        aboutCachelock.Lock(__FILE__, __LINE__);
        std::map<qcc::String, ApplicationMetaData>::const_iterator itr = aboutCache.find(app.busName);

        if (itr == aboutCache.end()) {
            cout << "Application with busname (" << app.busName.c_str() <<
                ") does not have cached about data!\nUpdating just the user defined name." << endl;
        } else {
            appMetaData.deviceName = itr->second.deviceName;
            appMetaData.appName = itr->second.appName;
        }
        aboutCachelock.Unlock(__FILE__, __LINE__);
    }

    do {
        ApplicationMetaData storedAppMetaData;
        if (ER_OK != uiStorage->GetAppMetaData(app, storedAppMetaData)) {
            cerr << "Failed to fetch persisted application meta data." << endl;
            break;
        }

        if (storedAppMetaData == appMetaData) {
            cout << "Application name and About meta data are already up to date..." << endl;
            break;
        }

        if (appMetaData.appName.empty()) {
            appMetaData.appName = storedAppMetaData.appName;
        }
        if (appMetaData.userDefinedName.empty()) {
            appMetaData.userDefinedName = storedAppMetaData.userDefinedName;
        }
        if (appMetaData.deviceName.empty()) {
            appMetaData.deviceName = storedAppMetaData.deviceName;
        }
        if (ER_OK != uiStorage->SetAppMetaData(app, appMetaData)) {
            cerr << "Failed to persist application name and/or About meta data." << endl;
        } else {
            cout << "Successfully persisted application name and/or About meta data." << endl;
        }
    } while (0);
}

static void add_group(const shared_ptr<UIStorage>& uiStorage,
                      const string& arg)
{
    vector<string> args = split(arg, '/');

    if (args.size() < 2) {
        cerr << "Please provide a group name and a description." << endl;
        return;
    }

    GroupInfo groupInfo;
    groupInfo.name = args[0].c_str();
    groupInfo.desc = args[1].substr(0, GROUP_DESC_MAX).c_str();

    if (ER_OK != uiStorage->StoreGroup(groupInfo)) {
        cerr << "Group was not added" << endl;
    } else {
        cout << "Group was successfully added" << endl;
        cout << groupInfo << endl;
    }
}

static void get_group(const shared_ptr<UIStorage>& uiStorage,
                      const string& arg)
{
    if (arg.empty()) {
        cout << "Empty group information" << endl;
        return;
    }

    GroupInfo groupInfo;
    GUID128 groupID(arg.substr(0, GROUP_ID_MAX).c_str());
    groupInfo.guid = groupID;

    if (ER_OK != uiStorage->GetGroup(groupInfo)) {
        cerr << "Group was not found" << endl;
    } else {
        cout << "Group was successfully retrieved" << endl;
        cout << groupInfo << endl;
    }
}

static void list_groups(const shared_ptr<UIStorage>& uiStorage)
{
    vector<GroupInfo> groups;

    if (ER_OK != uiStorage->GetGroups(groups)) {
        cerr << "Could not retrieve Groups or none were found" << endl;
    } else {
        cout << "Retrieved Group(s):" << endl;
        for (vector<GroupInfo>::const_iterator g = groups.begin();
             g != groups.end(); g++) {
            cout << *g << endl;
        }
    }
}

static void remove_group(const shared_ptr<UIStorage>& uiStorage,
                         const string& arg)
{
    if (arg.empty()) {
        cout << "Empty group information" << endl;
        return;
    }

    GUID128 groupID(arg.substr(0, GROUP_ID_MAX).c_str());

    GroupInfo group;
    group.guid = groupID;

    if (ER_OK != uiStorage->RemoveGroup(group)) {
        cerr << "Group was not found" << endl;
    } else {
        cout << "Group was successfully removed" << endl;
    }
}

static void update_membership(const shared_ptr<UIStorage>& uiStorage,
                              const string& arg, bool add)
{
    std::size_t delpos = arg.find_first_of(" ");

    if (arg.empty() || delpos == string::npos) {
        cerr << "Please provide an application id and group id." << endl;
        return;
    }

    qcc::String id = arg.substr(0, delpos).c_str();
    ajn::securitymgr::OnlineApplication app;

    if (!getKey(arg.substr(0, delpos), app.publicKey) || ER_END_OF_DATA == uiStorage->GetManagedApplication(
            app)) {
        cerr << "Could not find application with id " << id << "." << endl;
        return;
    }

    GroupInfo groupInfo;
    groupInfo.guid = GUID128(arg.substr(delpos + 1, string::npos).c_str());

    if (ER_OK != uiStorage->GetGroup(groupInfo)) {
        cerr << "Could not find group with id " << groupInfo.guid.ToString() << "." << endl;
        return;
    }

    if (add) {
        uiStorage->InstallMembership(app, groupInfo);
    } else {
        uiStorage->RemoveMembership(app, groupInfo);
    }
}

static void install_policy(const shared_ptr<UIStorage>& uiStorage,
                           const string& arg)
{
    vector<string> args = split(arg, ' ');

    if (args.size() < 1) {
        cerr << "Please provide an application id." << endl;
        return;
    }

    OnlineApplication app;
    if (!getKey(args[0], app.publicKey) || ER_END_OF_DATA == uiStorage->GetManagedApplication(app)) {
        cerr << "Could not find application." << endl;
        return;
    }

    vector<GroupInfo> groups;
    for (size_t i = 1; i < args.size(); ++i) {
        GUID128 groupID(args[i].c_str());
        GroupInfo groupInfo;
        groupInfo.guid = groupID;
        if (ER_OK != uiStorage->GetGroup(groupInfo)) {
            cerr << "Could not find group with id " << args[i] << endl;
            return;
        }
        groups.push_back(groupInfo);
    }

    PermissionPolicy policy;
    if (ER_OK != PolicyGenerator::DefaultPolicy(groups, policy)) {
        cerr << "Failed to generate default policy." << endl;
        return;
    }

    if (ER_OK != uiStorage->UpdatePolicy(app, policy)) {
        cerr << "Failed to install policy." << endl;
        return;
    }
    cout << "Successfully installed policy." << endl;
}

static void get_policy(const shared_ptr<UIStorage>& uiStorage,
                       const string& arg)
{
    vector<string> args = split(arg, ' ');

    if (args.size() < 1) {
        cerr << "Please provide an application id." << endl;
        return;
    }

    OnlineApplication app;
    if (!getKey(args[0], app.publicKey) || ER_END_OF_DATA == uiStorage->GetManagedApplication(app)) {
        cerr << "Could not find application." << endl;
        return;
    }
    PermissionPolicy policyLocal;

    if (ER_OK != uiStorage->GetPolicy(app, policyLocal)) {
        cerr << "Failed to get locally persisted policy." << endl;
        return;
    }

    cout << "Successfully retrieved locally persisted policy for " << args[0] << ":" << endl;
    cout << policyLocal.ToString() << endl;
}

static void help()
{
    cout << endl;
    cout << "  Supported commands:" << endl;
    cout << "  ===================" << endl;
    cout << "    q   Quit" << endl;
    cout << "    f   List all claimable applications" << endl;
    cout << "    c   Claim an application (appId)" << endl;
    cout << "    l   List all claimed applications" << endl;
    cout << "    g   Create a group (name/description)" << endl;
    cout << "    r   Remove a group (id)" << endl;
    cout << "    k   Get a group (id)" << endl;
    cout << "    p   List all groups" << endl;
    cout << "    m   Install a membership certificate (appId groupid)" << endl;
    cout << "    d   Delete a membership certificate (appId groupid)" << endl;
    cout << "    o   Install a policy (appId groupid1 groupid2 ...)" << endl;
    cout << "    e   Get policy (appId)" << endl;
    cout << "    u   Unclaim an application (appId)" << endl;
    cout << "    n   Set a user defined name for an application (appId appname)." << endl <<
        "        This operation will also persist relevant About meta data if they exist." << endl;
    cout << "    h   Show this help message" << endl << endl;
}

static bool parse(ajn::securitymgr::SecurityAgent* secAgent,
                  const shared_ptr<UIStorage>& uiStorage,
                  const string& input)
{
    char cmd;
    size_t argpos;
    string arg = "";

    if (input.length() == 0) {
        return true;
    }

    cmd = input[0];
    argpos = input.find_first_not_of(" \t", 1);
    if (argpos != input.npos) {
        arg = input.substr(argpos);
    }

    switch (cmd) {
    case 'q':
        return false;

    case 'f':
        list_claimable_applications(secAgent);
        break;

    case 'l':
        list_claimed_applications(uiStorage);
        break;

    case 'c':
        claim_application(secAgent, uiStorage, arg);
        break;

    case 'g':
        add_group(uiStorage, arg);
        break;

    case 'k':
        get_group(uiStorage, arg);
        break;

    case 'r':
        remove_group(uiStorage, arg);
        break;

    case 'p':
        list_groups(uiStorage);
        break;

    case 'm':
        update_membership(uiStorage, arg, true);
        break;

    case 'd':
        update_membership(uiStorage, arg, false);
        break;

    case 'o':
        install_policy(uiStorage, arg);
        break;

    case 'e':
        get_policy(uiStorage, arg);
        break;

    case 'u':
        unclaim_application(uiStorage, arg);
        break;

    case 'n':
        set_app_meta_data_and_name(uiStorage, secAgent, arg);
        break;

    case 'h':
    default:
        help();
        break;
    }

    return true;
}

int main(int argc, char** argv)
{
    QCC_UNUSED(argc);
    QCC_UNUSED(argv);

    cout << "\n\n\n";
    cout << "\t##########################################################" << endl;
    cout << "\t#    _____                               _   _           #" << endl;
    cout << "\t#   (_____)   ____                 _    (_) (_)_         #" << endl;
    cout << "\t#  (_)___    (____)    ___  _   _ (_)__  _  (___) _   _  #" << endl;
    cout << "\t#    (___)_ (_)()(_)  (___)(_) (_)(____)(_) (_)  (_) (_) #" << endl;
    cout << "\t#    ____(_)(__)__  (_)___ (_)_(_)(_)   (_) (_)_ (_)_(_) #" << endl;
    cout << "\t#   (_____)  (____)  (____) (___) (_)   (_)  (__) (____) #" << endl;
    cout << "\t#                                                 __ (_) #" << endl;
    cout << "\t#                                                (___)   #" << endl;
    cout << "\t#                                                        #" << endl;
    cout << "\t#          _____                          _              #" << endl;
    cout << "\t#         (_____)          ____    _     (_)_            #" << endl;
    cout << "\t#        (_)___(_)  ____  (____)  (_)__  (___)           #" << endl;
    cout << "\t#        (_______) (____)(_)()(_) (____) (_)             #" << endl;
    cout << "\t#        (_)   (_)( )_(_)(__)__   (_) (_)(_)_            #" << endl;
    cout << "\t#        (_)   (_) (____) (____)  (_) (_) (__)           #" << endl;
    cout << "\t#                 (_)_(_)                                #" << endl;
    cout << "\t#                  (___)                                 #" << endl;
    cout << "\t#                                                        #" << endl;
    cout << "\t##########   Type h to display the help menu  ############" << endl;
    cout << "\n\n\n";

    if (AllJoynInit() != ER_OK) {
        return EXIT_FAILURE;
    }
#ifdef ROUTER
    if (AllJoynRouterInit() != ER_OK) {
        AllJoynShutdown();
        return EXIT_FAILURE;
    }
#endif

    SecurityAgentIdentityInfo secAgentIdInfo;
    secAgentIdInfo.name = "Security Manager CLI Agent";
    secAgentIdInfo.vendor = "AllSeen Alliance";
    secAgentIdInfo.version = "2.0";

    SQLStorageFactory& storageFactory = SQLStorageFactory::GetInstance();
    shared_ptr<CaStorage> caStorage;
    shared_ptr<UIStorage> uiStorage;
    storageFactory.GetStorages("admin", caStorage, uiStorage);

    BusAttachment* ba = new BusAttachment("Security Agent", true);
    ba->Start();
    ba->Connect();

    CliAboutListner* cliAboutListener = new CliAboutListner();
    ba->RegisterAboutListener(*cliAboutListener);

    /* Passing NULL into WhoImplements will listen for all About announcements */
    if (ER_OK != ba->WhoImplements(NULL)) {
        cerr << "WhoImplements call FAILED\n";
        return EXIT_FAILURE;
    }

    SecurityAgentFactory& secFac = SecurityAgentFactory::GetInstance();
    SecurityAgent* secAgent = secFac.GetSecurityAgent(secAgentIdInfo, caStorage, ba);

    if (NULL == secAgent) {
        cerr
            << "> Error: Security Factory returned an invalid SecurityManager object !!"
            << endl;
        cerr << "> Exiting" << endl << endl;
        return EXIT_FAILURE;
    }

    secAgent->SetManifestListener(new CLIManifestListener());

    // Activate live monitoring
    EventListener listener;
    secAgent->RegisterApplicationListener(&listener);
    vector<IdentityInfo> list;
    if (ER_OK != uiStorage->GetIdentities(list)) {
        cerr
            << "> Error: Failed to retrieve identities !!"
            << endl;
        cerr << "> Exiting" << endl << endl;
    }
    if (list.size() == 0) {
        IdentityInfo info;
        info.guid = qcc::String("abcdef1234567890");
        info.name = "MyTestIdentity";
        if (ER_OK != uiStorage->StoreIdentity(info)) {
            cerr
                << "> Error: Failed to store default identity !!"
                << endl;
            cerr << "> Exiting" << endl << endl;
        }
    }

    bool done = false;
    while (!done) {
        string input;
        cout << "> ";
        getline(cin, input);
        done = !parse(secAgent, uiStorage, input);
    }

    // Cleanup
    ba->UnregisterAboutListener(*cliAboutListener);
    delete cliAboutListener;
    secAgent->UnregisterApplicationListener(&listener);
    ba->Disconnect();
    ba->Stop();

#ifdef ROUTER
    AllJoynRouterShutdown();
#endif
    AllJoynShutdown();
    return EXIT_SUCCESS;
}
