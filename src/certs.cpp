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
 * @file        certs.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation of certificates logic
 *              Getting out app signature, getting certificates out of
 *              signature. Checking OCSP
 */
#include <sys/types.h>
#include <dirent.h>
#include <list>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <vcore/SignatureValidator.h>
#include <vcore/SignatureFinder.h>
#include <vcore/Certificate.h>
#include <ckm/ckm-type.h>
#include <ckm/ckm-raw-buffer.h>
#include <tzplatform_config.h>

#include <cchecker/certs.h>
#include <cchecker/log.h>

namespace CCHECKER {

namespace {
struct PkgmgrinfoCertInfo {
    PkgmgrinfoCertInfo()
    {
        ret = pkgmgrinfo_pkginfo_create_certinfo(&handle);
    }
    ~PkgmgrinfoCertInfo()
    {
        pkgmgrinfo_pkginfo_destroy_certinfo(handle);
    }

    pkgmgrinfo_certinfo_h handle;
    int ret;
};

static void get_cert_chain(const char *pkgid, uid_t uid, int sig_type, chain_t &chain)
{
    LogDebug("Get cert chain start. pkgid : " << pkgid << ", uid : " << uid);
    int ret;
    int cert_type;
    const char *cert_value;

    auto pm_certinfo = std::make_shared<PkgmgrinfoCertInfo>();

    if (pm_certinfo->ret != PMINFO_R_OK) {
        LogError("Get pkgmgrinfo certinfo failed. ret : " << ret);
        return;
    }

    ret = pkgmgrinfo_pkginfo_load_certinfo(pkgid, pm_certinfo->handle, uid);
    if (ret != PMINFO_R_OK) {
        LogError("Load pkgmgrinfo certinfo failed. ret : " << ret);
        return;
    }

    // add signer, intermediate, root certificates.
    for (int cert_cnt = 0; cert_cnt < 3; cert_cnt++) {
        cert_type = sig_type - cert_cnt;
        ret = pkgmgrinfo_pkginfo_get_cert_value(pm_certinfo->handle,
                static_cast<pkgmgrinfo_cert_type>(cert_type), &cert_value);

        if (ret != PMINFO_R_OK) {
            LogError("Get cert value from certinfo failed. ret : " << ret);
            return;
        }

        if (cert_value == NULL) {
            LogDebug("cert_type[" << cert_type << "] is null");
        } else {
            LogDebug("Add cert_type[" << cert_type  << "] data : " << cert_value);
            chain.push_back(cert_value);
        }
    }

    return;
}
}

Certs::Certs()
{
    m_ckm = CKM::Manager::create();
}

Certs::~Certs()
{}

void Certs::get_certificates (app_t &app)
{
    // build chain using pkgmgr-info
    std::map<int, int> sig_type;
    sig_type[AUTHOR_SIG] = PMINFO_AUTHOR_SIGNER_CERT;
    sig_type[DISTRIBUTOR_SIG] = PMINFO_DISTRIBUTOR_SIGNER_CERT;
    sig_type[DISTRIBUTOR2_SIG] = PMINFO_DISTRIBUTOR2_SIGNER_CERT;

    for (auto s : sig_type) {
        chain_t chain;
        get_cert_chain(app.pkg_id.c_str(), app.uid, s.second, chain);

        if(!chain.empty()) {
            LogDebug("Add certificates chain to app. Size of chain : " << chain.size());
            app.signatures.emplace_back(std::move(chain));
        }
    }
}

Certs::ocsp_response_t Certs::check_ocsp_chain (const chain_t &chain)
{
    CKM::CertificateShPtrVector vect_ckm_chain;

    LogDebug("Size of chain: " << chain.size());
    for (auto &iter : chain) {
        CKM::RawBuffer buff(iter.begin(), iter.end());
        auto cert = CKM::Certificate::create(buff, CKM::DataFormat::FORM_DER_BASE64);
        vect_ckm_chain.emplace_back(std::move(cert));
    }

    int status = CKM_API_OCSP_STATUS_UNKNOWN;
    int ret = m_ckm->ocspCheck(vect_ckm_chain, status);
    if (ret != CKM_API_SUCCESS) {
        LogError("CKM ckeck OCSP returned " << ret);
        // Add handling for different errors codes
        // For these we can try to check ocsp again later:
        switch (ret) {
        case CKM_API_ERROR_NOT_SUPPORTED:
            LogDebug("Key-manager OCSP API temporary diabled.");
        case CKM_API_ERROR_SOCKET:
        case CKM_API_ERROR_BAD_REQUEST:
        case CKM_API_ERROR_BAD_RESPONSE:
        case CKM_API_ERROR_SEND_FAILED:
        case CKM_API_ERROR_RECV_FAILED:
        case CKM_API_ERROR_SERVER_ERROR:
        case CKM_API_ERROR_OUT_OF_MEMORY:
            return Certs::ocsp_response_t::OCSP_CHECK_AGAIN;
        // Any other error should be recurrent - checking the same app again
        // should give the same result.
        default:
            return Certs::ocsp_response_t::OCSP_CERT_ERROR;
        }
    }

    LogDebug("OCSP status: " << status);
    switch (status) {
    // Remove app from "to-check" list:
    case CKM_API_OCSP_STATUS_GOOD:
        return Certs::ocsp_response_t::OCSP_APP_OK;
    case CKM_API_OCSP_STATUS_UNSUPPORTED:
    case CKM_API_OCSP_STATUS_UNKNOWN:
    case CKM_API_OCSP_STATUS_INVALID_URL:
        return Certs::ocsp_response_t::OCSP_CERT_ERROR;

    //Show popup to user and remove app from "to-check" list
    case CKM_API_OCSP_STATUS_REVOKED:
        return Certs::ocsp_response_t::OCSP_APP_REVOKED;

     //Keep app for checking it again later:
    case CKM_API_OCSP_STATUS_NET_ERROR:
    case CKM_API_OCSP_STATUS_INVALID_RESPONSE:
    case CKM_API_OCSP_STATUS_REMOTE_ERROR:
    case CKM_API_OCSP_STATUS_INTERNAL_ERROR:
        return Certs::ocsp_response_t::OCSP_CHECK_AGAIN;

    default:
        // This should never happen
        return Certs::ocsp_response_t::OCSP_CERT_ERROR;
    }
}

Certs::ocsp_response_t Certs::check_ocsp (const app_t &app)
{
    bool check_again = false;

    for (auto &iter : app.signatures) {
        Certs::ocsp_response_t resp = check_ocsp_chain(iter);
        if (resp == Certs::ocsp_response_t::OCSP_APP_REVOKED)
            return Certs::ocsp_response_t::OCSP_APP_REVOKED;
        if (resp == Certs::ocsp_response_t::OCSP_CHECK_AGAIN)
           check_again = true;
    }

    if (check_again)
        return Certs::ocsp_response_t::OCSP_CHECK_AGAIN;
    return Certs::ocsp_response_t::OCSP_APP_OK;
}

} // CCHECKER
