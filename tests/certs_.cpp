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
 * @file        dbfixture.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Implementation of DB test class
 */

#include <certs_.h>

using namespace CCHECKER;

Certs_::~Certs_()
{}

Certs_::ocsp_response_t Certs_::check_ocsp_chain_ (const chain_t &chain)
{
    return this->check_ocsp_chain(chain);
}

void Certs_::find_app_signatures_ (app_t &app, const std::string &app_path, ocsp_urls_t &ocsp_urls)
{
    return this->find_app_signatures(app, app_path, ocsp_urls);
}

void Certs_::search_app_ (app_t &app, ocsp_urls_t &ocsp_urls)
{
    return this->search_app(app, ocsp_urls);
}
