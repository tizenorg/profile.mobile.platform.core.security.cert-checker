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
#include <vector>
#include <sys/types.h>

#include <app.h>

namespace CCHECKER {

app_t & app_t::operator= (const app_t &app)
{
    app_id = app.app_id;
    pkg_id = app.pkg_id;
    uid = app.uid;
    check_id = app.check_id;
    verified = app.verified;
    certificates = app.certificates;

    return *this;
}

std::ostream & operator<< (std::ostream &out, const app_t &app)
{
    out << "app: " << app.app_id << ", pkg: " << app.pkg_id << ", uid: " << app.uid;
    return out;
}

const std::string app_t::str() const
{
    std::stringstream ss;
    ss << this;
    const std::string ret = ss.str();
    return ret;
}

} //CCHECKER
