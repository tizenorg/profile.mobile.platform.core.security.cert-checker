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
 * @file        app.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation of app struct
 */
#ifndef CCHECKER_APP_H
#define CCHECKER_APP_H

#include <string>
#include <list>
#include <sys/types.h>

namespace CCHECKER {

// Used as app_id when no information about app id in signal
// Currently in signals from pkgmgr only information about pkg_id is included
const char *const TEMP_APP_ID = "temp#app_id";

typedef std::list<std::string> chain_t;
typedef std::list<chain_t> signatures_t;

struct app_t {
    enum class verified_t : int32_t {
        NO      = 0,
        YES     = 1,
        UNKNOWN = 2
    };

    int32_t        check_id;
    std::string    app_id;
    std::string    pkg_id;
    uid_t          uid;
    signatures_t   signatures;
    verified_t     verified;

    app_t(void);
    app_t(const std::string &app_id,
          const std::string &pkg_id,
          uid_t uid,
          const signatures_t &signatures);
    std::string str(void) const;
    std::string str_certs(void) const;
};

struct url_t {
    std::string issuer;
    std::string url;
    int64_t     date;

    url_t(const std::string &_issuer,
          const std::string &_url,
          int64_t _date):
              issuer(_issuer),
              url(_url),
              date(_date)
    {};
};

typedef std::list<url_t> ocsp_urls_t;

} //CCHECKER

#endif //CCHECKER_APP_H
