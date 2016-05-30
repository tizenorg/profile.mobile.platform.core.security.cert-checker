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
 * @file        dbfixture.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Implementation of DB test class
 */

#include <cchecker/sql_query.h>

#ifndef CCHECKER_DBFIXTURE_H
#define CCHECKER_DBFIXTURE_H

using namespace CCHECKER;

class DBFixture : public DB::SqlQuery {
public:
	DBFixture();
	virtual ~DBFixture();
};

#endif //CCHECKER_DBFIXTURE_H
