/*
 *  Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License
 */
/*
 * @file        popup.h
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief
 */
#pragma once

#include <Elementary.h>
#include <string>
#include <vector>
#include <memory>

#include "service/app.h"

namespace CCHECKER {
namespace UI {

enum class ResponseType : int {
	KEEP = 0x01,
	UNINSTALL = 0x02
};

class Popup {
public:
	explicit Popup(const app_t &app);
	virtual ~Popup();

	Popup(const Popup &) = delete;
	Popup &operator=(const Popup &) = delete;
	Popup(Popup &&) = delete;
	Popup &operator=(Popup &&) = delete;

	void run(void);

private:
	void callbackRegister(Evas_Object *obj, ResponseType *type);
	static void btnClickedCb(void *data, Evas_Object *, void *);

	void setDefaultProperties(Evas_Object *obj) noexcept;
	void setText(Evas_Object *obj, const std::string &text) noexcept;

	Evas_Object *m_win;
	Evas_Object *m_popup;
	Evas_Object *m_box;
	Evas_Object *m_title;
	Evas_Object *m_content;
	Evas_Object *m_keepBtn;
	Evas_Object *m_uninstallBtn;

	static int response;

	ResponseType m_keepType;
	ResponseType m_uninstallType;

	app_t m_app;
};

} // namespace UI
} // namespace CCHECKER 
