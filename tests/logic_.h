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
 * @file        logic_.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       This file is the tesst implementation of Logic class
 */

#ifndef CCHECKER_LOGIC__H
#define CCHECKER_LOGIC__H

/*
 * This Class makes all methods from Logic Class public - for testing purpose.
 * Some of methods are stubbed, and some of them just calls corresponding methods from Logic Class.
 */

#include <cchecker/logic.h>

namespace CCHECKER {

class Logic_ : public Logic {
    public:
        Logic_(void);
        virtual ~Logic_(void);

        // For tests only
        void connman_callback_manual_(bool state);
        void pkgmgr_install_manual_(const app_t &app);
        void pkgmgr_uninstall_manual_(const app_t &app);
        const std::list<app_t>& get_buffer_();
};

} // CCHECKER

#endif //CCHECKER_LOGIC__H
