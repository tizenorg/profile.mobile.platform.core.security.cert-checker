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
 * @file        app.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation of app struct
 */

#include <sstream>
#include <string>
#include <sys/types.h>

#include <cchecker/app.h>
#include <cchecker/log.h>

namespace CCHECKER {

void certificates_chain::sort(void)
{
    certificates.sort();
}

bool certificates_chain::operator ==(const certificates_chain &chain) const
{
    return certificates == chain.certificates;
}

bool certificates_chain::operator !=(const certificates_chain &chain) const
{
    return certificates != chain.certificates;
}

bool certificates_chain::operator <(const certificates_chain &chain) const
{
    return certificates < chain.certificates;
}

bool certificates_chain::operator <=(const certificates_chain &chain) const
{
    return certificates <= chain.certificates;
}

bool certificates_chain::operator >(const certificates_chain &chain) const
{
    return certificates > chain.certificates;
}

bool certificates_chain::operator >=(const certificates_chain &chain) const
{
    return certificates >= chain.certificates;
}

app_t::app_t(void):
    check_id(-1),   // -1 as invalid check_id - assume that in database
                    // all check_ids will be positive
    uid((uid_t)-1), // (uid_t)-1 (0xFF) is defined to be invalid uid. According
                    // to chown manual page, you cannot change file group of owner
                    // to (uid_t)-1, so we'll use it as initial, invalid value.
    verified(verified_t::UNKNOWN)
{}

app_t::app_t(const std::string &app_id,
        const std::string &pkg_id,
        uid_t uid,
        const certificates_t &certificates):
    check_id(-1),
    app_id(app_id),
    pkg_id(pkg_id),
    uid(uid),
    certificates(certificates),
    verified(verified_t::UNKNOWN)
{}

std::ostream & operator<< (std::ostream &out, const app_t &app)
{
    out << "app: " << app.app_id << ", pkg: " << app.pkg_id << ", uid: " << app.uid <<
            ", check_id: " << app.check_id;
    return out;
}

std::string app_t::str() const
{
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

std::string app_t::str_certs(void) const
{
    std::stringstream ss;

    for (const auto &iter : certificates) {
        ss << "<Chain " << iter.chain_id << ">: ";
        for (const auto iter_cert : iter.certificates) {
            ss << "\"" << iter_cert << "\", ";
        }
    }
    return ss.str();
}

void app_t::sort(void)
{
    for(auto &iter : certificates) {
        iter.certificates.sort();
    }
    certificates.sort();
}

bool app_t::compare (const app_t &app) const
{
    if (app_id != app.app_id || // do not compare check_id - it is for database internal use only
            pkg_id != app.pkg_id ||
            uid != app.uid ||
            certificates.size() != app.certificates.size() ||
            verified != app.verified) {
        LogDebug("app_t compare error: " << str() << " is different than: " << app.str());
        return false;
    }
    return true;
}

bool app_t::operator ==(const app_t &app) const
{
    if (!compare(app))
        return false;
    return certificates == app.certificates;
}

bool app_t::operator !=(const app_t &app) const
{
    return !(*this == app);
}

} //CCHECKER
