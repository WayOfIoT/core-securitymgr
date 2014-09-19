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

#include <alljoyn/BusAttachment.h>
#include <alljoyn/BusObject.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include "Stub.h"
#include "MyClaimListener.h"

using namespace ajn;
using namespace services;

void printHelp()
{
    std::cout << "Welcome to the permission mgmt stub" << std::endl
              << "Menu" << std::endl
              << ">o : Opens the claim window" << std::endl
              << ">c : Closes the claim window" << std::endl
              << ">i : Lists identity certificates" << std::endl
              << ">r : Lists RoT's" << std::endl
              << ">s : Send Signal" << std::endl
              << ">q : Quit" << std::endl;
}

int main(int arg, char** argv)
{
    char c;
    srand(time(NULL) + (int)getpid());

    MyClaimListener mycl;
    Stub stub(&mycl);
    printHelp();

    while ((c = std::cin.get()) != 'q') {
        switch (c) {
        case 'h':
            {
                printHelp();
                break;
            }

        case 'o':
            {
                QStatus status = stub.OpenClaimWindow();
                if (status != ER_OK) {
                    std::cerr << "Could not open claim window " << QCC_StatusText(status) << std::endl;
                    break;
                }

                break;
            }

        case 'c':
            {
                QStatus status = stub.CloseClaimWindow();
                if (status != ER_OK) {
                    std::cerr << "Could not close claim window " << QCC_StatusText(status) << std::endl;
                    break;
                }

                break;
            }

        case 'i':
            {
                qcc::String identityCert = stub.GetInstalledIdentityCertificate();
                if (qcc::String("") == identityCert) {
                    printf("There are currently no Identity certificates installed \n");
                } else {
                    printf("Installed Identity Certificate: %s \n", identityCert.c_str());
                }
                break;
            }

        case 'r':
            {
                std::vector<qcc::ECCPublicKey*> publicRoTKeys = stub.GetRoTKeys();
                if (0 != publicRoTKeys.size()) {
                    // Printout valid RoT pub key
                    for (std::vector<qcc::ECCPublicKey*>::const_iterator it = publicRoTKeys.begin();
                         it != publicRoTKeys.end();
                         ++it) {
                        std::string printableRoTKey = "";
                        for (int i = 0; i < (int)qcc::ECC_PUBLIC_KEY_SZ; ++i) {
                            char buff[4];
                            sprintf(buff, "%02x", (unsigned char)((*it)->data[i]));
                            printableRoTKey = printableRoTKey + buff;
                        }
                        printf("RoT pubKey: %s \n", printableRoTKey.c_str());
                    }
                } else {
                    printf("There are currently no Root of Trust certificates installed \n");
                }
                break;
            }

        case 's':
            {
                QStatus status = stub.SendClaimDataSignal();
                if (status != ER_OK) {
                    std::cerr << "Could not send secInfo " << QCC_StatusText(status) << std::endl;
                }
                break;
            }

        case 'q':
        case '\r':
        case '\n':
            break;

        default:
            std::cerr << "Unknown option" << std::endl;
        }
    }
}