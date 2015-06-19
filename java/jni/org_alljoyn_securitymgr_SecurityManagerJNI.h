/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_alljoyn_securitymgr_SecurityManagerJNI */

#ifndef _Included_org_alljoyn_securitymgr_SecurityManagerJNI
#define _Included_org_alljoyn_securitymgr_SecurityManagerJNI
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    initJNI
 * Signature: (Ljava/lang/Class;Ljava/lang/Class;Ljava/lang/Class;)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_initJNI
    (JNIEnv*, jclass, jclass, jclass, jclass);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    init
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_init
    (JNIEnv*, jobject, jstring);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    unclaimApplication
 * Signature: (Lorg/alljoyn/securitymgr/Application;)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_unclaimApplication
    (JNIEnv*, jobject, jobject);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    getApplications
 * Signature: (Ljava/util/List;)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_getApplications
    (JNIEnv*, jobject, jobject);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    createGuild
 * Signature: (Ljava/lang/String;Ljava/lang/String;[B[B)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_createGuild
    (JNIEnv*, jobject, jstring, jstring, jbyteArray, jbyteArray);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    deleteGuild
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_deleteGuild
    (JNIEnv*, jobject, jbyteArray);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    getGuild
 * Signature: ([BLjava/lang/Class;)Lorg/alljoyn/securitymgr/Guild;
 */
JNIEXPORT jobject JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_getGuild
    (JNIEnv*, jobject, jbyteArray, jclass);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    getGuilds
 * Signature: (Ljava/util/List;Ljava/lang/Class;)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_getGuilds
    (JNIEnv*, jobject, jobject, jclass);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    createIdentity
 * Signature: (Ljava/lang/String;[BZ)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_createIdentity
    (JNIEnv*, jobject, jstring, jbyteArray, jbyteArray);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    deleteIdentity
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_deleteIdentity
    (JNIEnv*, jobject, jbyteArray);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    getIdentity
 * Signature: ([B)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_getIdentity
    (JNIEnv*, jobject, jbyteArray);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    getIdentities
 * Signature: (Ljava/util/List;Ljava/lang/Class;)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_getIdentities
    (JNIEnv*, jobject, jobject, jclass);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    installPolicy
 * Signature: (Lorg/alljoyn/securitymgr/Application;J[Lorg/alljoyn/securitymgr/access/Term;)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_installPolicy
    (JNIEnv*, jobject, jobject, jlong, jobjectArray);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    installMembership
 * Signature: (Lorg/alljoyn/securitymgr/Application;[B[B[Lorg/alljoyn/securitymgr/access/Term;)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_installMembership
    (JNIEnv*, jobject, jobject, jbyteArray, jbyteArray, jobjectArray);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    deleteMembership
 * Signature: (Lorg/alljoyn/securitymgr/Application;[B[B)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_deleteMembership
    (JNIEnv*, jobject, jobject, jbyteArray, jbyteArray);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    claimApplication
 * Signature: (Lorg/alljoyn/securitymgr/Application;[B)V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_claimApplication
    (JNIEnv*, jobject, jobject, jbyteArray, jbyteArray);

/*
 * Class:     org_alljoyn_securitymgr_SecurityManagerJNI
 * Method:    getPubicKey
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_alljoyn_securitymgr_SecurityManagerJNI_getPublicKey
    (JNIEnv*, jobject);

#ifdef __cplusplus
}
#endif
#endif
