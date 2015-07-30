/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        stubs_.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Implementation of stubbed functions for testing
 */

#include <cchecker/certs.h>
#include <cchecker/queue.h>
#include <cchecker/sql_query.h>
#include <cchecker/UIBackend.h>

namespace CCHECKER {

// Certs

Certs::Certs()
{}

Certs::~Certs()
{}

void Certs::get_certificates (app_t &app, ocsp_urls_t &ocsp_urls)
{
    (void)app;
    (void)ocsp_urls;
}

Certs::ocsp_response_t Certs::check_ocsp_chain (const chain_t &chain)
{
    (void)chain;
    return ocsp_response_t::OCSP_APP_OK;
}

// Only the public functions need to be stubbed for testing all possibilities
Certs::ocsp_response_t Certs::check_ocsp (const app_t &app)
{
    if (app.signatures.empty())
        return ocsp_response_t::OCSP_APP_OK;

    if (app.signatures.front().empty())
            return ocsp_response_t::OCSP_APP_OK;

    std::string ret = app.signatures.front().front();

    if ("OCSP_APP_REVOKED" == ret)
        return ocsp_response_t::OCSP_APP_REVOKED;
    if ("OCSP_CHECK_AGAIN" == ret)
        return ocsp_response_t::OCSP_CHECK_AGAIN;
    if ("OCSP_CERT_ERROR" == ret)
        return ocsp_response_t::OCSP_CERT_ERROR;

    return ocsp_response_t::OCSP_APP_OK;
}


void Certs::find_app_signatures (app_t &app, const std::string &app_path, ocsp_urls_t &ocsp_urls)
{
    (void)app;
    (void)app_path;
    (void)ocsp_urls;
}

void Certs::search_app (app_t &app, ocsp_urls_t &ocsp_urls)
{
    (void)app;
    (void)ocsp_urls;
}

bool Certs::ocsp_create_list(const chain_t &chain, ValidationCore::CertificateList &certs_list)
{
    (void)chain;
    (void)certs_list;
    return true;
}

bool Certs::ocsp_build_chain (const ValidationCore::CertificateList &certs_list,
        CKM::CertificateShPtrVector &vect_ckm_chain)
{
    (void)certs_list;
    (void)vect_ckm_chain;
    return true;
}

// DB
DB::SqlQuery::SqlQuery(const std::string &path)
{
    m_connection = NULL;
    m_inUserTransaction = false;

    (void)path;
}

DB::SqlQuery::~SqlQuery()
{}

bool DB::SqlQuery::get_url(const std::string &issuer, std::string &url)
{
    (void)issuer;
    (void)url;
    return false;
}

void DB::SqlQuery::set_url(const std::string &issuer, const std::string &url, const int64_t &date)
{
    (void)issuer;
    (void)url;
    (void)date;
}

bool DB::SqlQuery::add_app_to_check_list(const app_t &app)
{
    (void)app;
    return true; // Success
}

void DB::SqlQuery::remove_app_from_check_list(const app_t &app)
{
    (void)app;
}

void DB::SqlQuery::mark_as_verified(const app_t &app, const app_t::verified_t &verified)
{
    (void)app;
    (void)verified;
}

void DB::SqlQuery::get_app_list(std::list<app_t> &apps_buffer)
{
    (void)apps_buffer;
}

// UI
UI::UIBackend::UIBackend(int timeout) :
    m_notification(nullptr),
    m_responseTimeout(timeout)
{}

UI::UIBackend::~UIBackend()
{}

bool UI::UIBackend::call_popup(const app_t &app)
{
    if (app.uid > 5000)
        return true;

    return false;
}

} //CCHECKER
