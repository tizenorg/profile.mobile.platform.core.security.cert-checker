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
#include <vcore/CertificateCollection.h>
#include <vcore/SignatureReader.h>
#include <vcore/SignatureFinder.h>
#include <vcore/WrtSignatureValidator.h>
#include <vcore/VCore.h>
#include <ckm/ckm-type.h>
#include <ckm/ckm-raw-buffer.h>

#include <cchecker/certs.h>
#include <cchecker/log.h>

namespace {
const std::string signatureXmlSchemaPath = std::string(tzplatform_getenv(TZ_SYS_SHARE))
        + std::string("/app-installers/signature_schema.xsd");
}

namespace CCHECKER {

Certs::Certs()
{
    ValidationCore::VCoreInit();
    m_ckm = CKM::Manager::create();
}

Certs::~Certs()
{
    ValidationCore::VCoreDeinit();
}

void Certs::get_certificates (app_t &app, ocsp_urls_t &ocsp_urls)
{
    std::vector<std::string> signatures;
    (void) signatures;

    if (0 != tzplatform_set_user(app.uid)) {
        LogError("Cannot set user: tzplatform_set_user has failed");
        return;
    }

    if (app.app_id == TEMP_APP_ID) {
        LogDebug("Temporary app_id. Searching for apps in package.");
        search_app(app, ocsp_urls);
    }
    else {
        const char *pkg_path = tzplatform_mkpath(TZ_USER_APP, app.pkg_id.c_str());
        std::string app_path = std::string(pkg_path) + std::string("/") + app.app_id;
        find_app_signatures (app, app_path, ocsp_urls);
    }

}

/* Since there's no information about application in signal,
 * and we've got information only about package, we have to check
 * all applications that belongs to that package
 */
void Certs::search_app (app_t &app, ocsp_urls_t &ocsp_urls)
{
    DIR *dp;
    struct dirent *entry;
    const char *pkg_path = tzplatform_mkpath(TZ_USER_APP, app.pkg_id.c_str());
    if (!pkg_path) {
        LogError("tzplatform_mkpath has returned NULL for TZ_USER_APP");
        return;
    }

    dp = opendir(pkg_path);
    if (dp != NULL) {
        while ((entry = readdir(dp))) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && entry->d_type == DT_DIR) {
                LogDebug("Found app: " << entry->d_name);
                std::string app_path = std::string(pkg_path) + std::string("/") + std::string(entry->d_name);
                find_app_signatures(app, app_path, ocsp_urls);
            }
        }
        closedir(dp); //close directory
    }
    else
        LogError("Couldn't open the package directory.");
}

// Together with certificates we can pull out OCSP URLs
void Certs::find_app_signatures (app_t &app, const std::string &app_path, ocsp_urls_t &ocsp_urls)
{
    ValidationCore::SignatureFinder signature_finder(app_path);
    ValidationCore::SignatureFileInfoSet signature_files;

    if (signature_finder.find(signature_files) !=
            ValidationCore::SignatureFinder::NO_ERROR) {
           LogError("Error while searching for signatures in " << app_path.c_str());
           return;
    }
    LogDebug("Number of signature files: " << signature_files.size());

    LogDebug("Searching for certificates");
    for (auto iter = signature_files.begin(); iter != signature_files.end(); iter++) {
        chain_t chain;
        LogDebug("Checking signature");
        ValidationCore::SignatureData data(app_path + std::string("/") + (*iter).getFileName(),
                (*iter).getFileNumber());
        LogDebug("signatureXmlSchemaPath: " << signatureXmlSchemaPath);
        try {
            ValidationCore::SignatureReader reader;
            reader.initialize(data, signatureXmlSchemaPath);
            reader.read(data);
            ValidationCore::CertificateList certs = data.getCertList();
            for (auto cert_iter = certs.begin(); cert_iter != certs.end(); cert_iter++ ){
                std::string app_cert = (*cert_iter)->getBase64();
                chain.push_back(app_cert);
                LogDebug("Certificate: " << app_cert << " has been added");

                // check OCSP URL
                std::string ocsp_url = DPL::ToUTF8String((*cert_iter)->getOCSPURL());
                if (ocsp_url != std::string("")) {
                    std::string issuer = DPL::ToUTF8String(
                            (*cert_iter)->getCommonName(ValidationCore::Certificate::FIELD_ISSUER));
                    int64_t time = (*cert_iter)->getNotBefore();
                    url_t url(issuer, ocsp_url, time);
                    ocsp_urls.push_back(url);
                    LogDebug("Found OCSP URL: " << ocsp_url << " for issuer: " <<  issuer << ", time: " << time);

                }
            }
        } catch (const ValidationCore::ParserSchemaException::Base& exception) {
            // Needs to catch parser exceptions
            LogError("Error occured in ParserSchema: " << exception.DumpToString());
        }
        if (!chain.empty()) {
            app.signatures.push_back(chain);
            LogDebug("Certificates chain added to the app");
        }
    }
}

bool Certs::ocsp_create_list (const chain_t &chain, ValidationCore::CertificateList &certs_list)
{
    ValidationCore::CertificateCollection collection;
    ValidationCore::CertificateList list;

    LogDebug("Chain size: " << chain.size());
    for (auto &iter : chain) {
        try {
            ValidationCore::CertificatePtr p_cert(
                    new ValidationCore::Certificate(iter, ValidationCore::Certificate::FORM_BASE64));
            list.push_back(p_cert);
        } catch (const ValidationCore::Certificate::Exception::Base& exception) {
            LogError("Error while creating certificate from BASE64: " << exception.DumpToString());
            return false;
        }
        LogDebug("Load certificate to list: " << list.size());
    }

    // Function collection.load which takes certificate in std::string BASE64 fails for some reason,
    // so load(const CertificateList &certList) is used.
    collection.load(list);
    LogDebug("Load certificate to CertificateCollection: " << collection.size());

    if (!collection.sort()) {
        LogError("Cannot make chain of certificates");
        // What to do if chain cannot be build?
        return false;
    }

    if (collection.isChain()) {
        LogDebug("Build chain succeed, size: " << collection.size());
    } else {
        LogError("Building chain failed");
        return false;
    }

    certs_list = collection.getCertificateList();

    return true;
}

bool Certs::ocsp_build_chain (const ValidationCore::CertificateList &certs_list, CKM::CertificateShPtrVector &vect_ckm_chain)
{
    CKM::CertificateShPtrVector vect_untrusted;

    bool first = true;
    CKM::CertificateShPtr cert_end_entity;
    LogDebug("Size of certs_list: " << certs_list.size());
    for (auto &iter : certs_list) {
        std::string cert_cp(iter->getBase64());
        CKM::RawBuffer buff(cert_cp.begin(), cert_cp.end());
        CKM::CertificateShPtr cert = CKM::Certificate::create(buff, CKM::DataFormat::FORM_DER_BASE64);

        if (!cert) {
            LogDebug("CKM failed to create certificate");
            return false;
        }
        else if (first) {
            cert_end_entity = cert;
            first = false;
            LogDebug("Found end entity certificate");
        }
        else {
            vect_untrusted.push_back(cert);
            LogDebug("Found untrusted certificate");
        }
    }

    int ret = m_ckm->getCertificateChain(
            cert_end_entity,
            vect_untrusted,
            CKM::CertificateShPtrVector(),
            true, // useTrustedSystemCertificates
            vect_ckm_chain);
    if (ret != CKM_API_SUCCESS) {
        LogError("CKM getCertificateChain returned: " << ret);
        // TODO: Add handling for different errors codes?
        return false;
    }

    return true;
}

Certs::ocsp_response_t Certs::check_ocsp_chain (const chain_t &chain)
{
    ValidationCore::CertificateList certs_list;
    if (!ocsp_create_list(chain, certs_list)) {
        LogError("Error while build list of certificates");
        return Certs::ocsp_response_t::OCSP_CERT_ERROR;
    }

    CKM::CertificateShPtrVector vect_ckm_chain;

    if (!ocsp_build_chain(certs_list, vect_ckm_chain)) {
        LogError("Error while build chain of certificates");
        return Certs::ocsp_response_t::OCSP_CERT_ERROR;
    }

    int status;
    int ret = m_ckm->ocspCheck(vect_ckm_chain, status);
    if (ret != CKM_API_SUCCESS) {
        LogError("CKM ckeck OCSP returned " << ret);
        // Add handling for different errors codes
        // For these we can try to check ocsp again later:
        switch (ret) {
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
