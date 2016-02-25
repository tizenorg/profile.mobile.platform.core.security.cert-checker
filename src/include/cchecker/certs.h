/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file        certs.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation of certificates logic
 *              Getting out findinf app signature, getting certificates out of
 *              signature. Checking OCSP
 */

#ifndef CCHECKER_CERTS_H
#define CCHECKER_CERTS_H

#include <ckm/ckm-certificate.h>
#include <vcore/Certificate.h>
#include <pkgmgr-info.h>

#include <cchecker/app.h>
#include <ckm/ckm-manager.h>

namespace CCHECKER {

enum sig_t {
    AUTHOR_SIG,
    DISTRIBUTOR_SIG,
    DISTRIBUTOR2_SIG
};

class Certs {
    public:
        enum class ocsp_response_t {
            OCSP_APP_OK,
            OCSP_APP_REVOKED,
            OCSP_CHECK_AGAIN,
            OCSP_CERT_ERROR
        };
        Certs();
        virtual ~Certs();
        void get_certificates (app_t &app);
        ocsp_response_t check_ocsp (const app_t &app);

    protected: // Needed for tests
        ocsp_response_t check_ocsp_chain (const chain_t &chain);

    //private:
        CKM::ManagerShPtr m_ckm;
};

} // CCHECKER


#endif // CCHECKER_CERTS_H
