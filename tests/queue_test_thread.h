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
 * @file        queue_test_thread.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Implementation of class for multi-thread testing Queue
 */

#include "service/queue.h"

namespace CCHECKER {

class TestQueue {
    public:
        TestQueue():
            m_sent(0),
            m_max_events(0)
        {};
        void add_events_th (int number_of_threads);
        bool pop_events ();

    private:
        Queue m_queue;
        int  m_sent;
        int  m_max_events;
        void add_events();
};

} // CCHECKER
