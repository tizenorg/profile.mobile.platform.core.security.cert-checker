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
 * @file        popup.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <libintl.h>
#include <sys/select.h>
#include <time.h>

#include <Elementary.h>

#include <popup.h>
#include "common/log.h"
#include <cchecker/UIBackend.h>
#include <cchecker/popup-runner.h>
#include <cchecker/dpl/serialization.h>
#include <cchecker/dpl/errno_string.h>
#include <cchecker/dpl/exception.h>

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif

using namespace CCHECKER::UI;

namespace { // anonymous

void on_done(void)
{
	// Quit the efl-mainloop
	LogDebug("elm_exit()");
	elm_exit();
}

void keep_answer(void *data, Evas_Object * /* obj */, void * /* event_info */)
{
	LogDebug("keep_answer");

	if (NULL == data) {
		LogError("data is NULL; return");
		return;
	}

	struct cert_checker_popup_data *pdp = static_cast <struct cert_checker_popup_data *>(data);

	pdp->result = response_e::DONT_UNINSTALL;

	on_done();
}

void uninstall_answer(void *data, Evas_Object * /* obj */, void * /* event_info */)
{
	LogDebug("uninstall_answer");

	if (NULL == data) {
		LogError("data is NULL; return");
		return;
	}

	struct cert_checker_popup_data *pdp = static_cast <struct cert_checker_popup_data *>(data);

	pdp->result = response_e::UNINSTALL;

	on_done();
}

void show_popup(struct cert_checker_popup_data *pdp)
{
	LogDebug("show_popup()");

	if (NULL == pdp) {
		LogError("pdp is NULL; return");
		return;
	}

	pdp->win = elm_win_add(NULL,
						   dgettext(SERVICE_NAME, "SID_TITLE_OCSP_VERIFICATION_FAILED"),
						   ELM_WIN_NOTIFICATION);
	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
	elm_win_autodel_set(pdp->win, EINA_TRUE);
	evas_object_show(pdp->win);
	elm_win_indicator_opacity_set(pdp->win, ELM_WIN_INDICATOR_TRANSLUCENT);
	pdp->popup = elm_popup_add(pdp->win);
	pdp->box = elm_box_add(pdp->popup);
	evas_object_size_hint_weight_set(pdp->box, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(pdp->box, EVAS_HINT_FILL, 0.0);
	pdp->title = elm_label_add(pdp->popup);
	elm_object_style_set(pdp->title, "elm.text.title");
	elm_object_text_set(pdp->title, dgettext(SERVICE_NAME, "SID_TITLE_OCSP_VERIFICATION_FAILED"));
	evas_object_show(pdp->title);
	elm_box_pack_end(pdp->box, pdp->title);
	pdp->content = elm_label_add(pdp->popup);
	elm_object_style_set(pdp->content, "elm.swallow.content");
	elm_label_line_wrap_set(pdp->content, ELM_WRAP_MIXED);
	char *buff = NULL;
	int ret;

	// Set message
	// App ID may be absent, so in that case we need to use only package ID
	if (pdp->app_id == std::string(CCHECKER::TEMP_APP_ID)) {
		char *content = dgettext(SERVICE_NAME, "SID_CONTENT_OCSP_PACKAGE VERIFICATION_FAILED");
		ret = asprintf(&buff, content, pdp->pkg_id.c_str());
	} else {
		char *content = dgettext(SERVICE_NAME, "SID_CONTENT_OCSP_VERIFICATION_FAILED");
		ret = asprintf(&buff, content, pdp->app_id.c_str(), pdp->pkg_id.c_str());
	}

	if (-1 == ret) {
		LogError("asprintf failed - returned -1");
		evas_object_del(pdp->content);
		evas_object_del(pdp->popup);
		evas_object_del(pdp->win);
		return;
	}

	elm_object_text_set(pdp->content, buff);
	LogDebug("Popup label: " << buff);
	free(buff);
	evas_object_size_hint_weight_set(pdp->content, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(pdp->content, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(pdp->content);
	elm_box_pack_end(pdp->box, pdp->content);
	elm_object_part_content_set(pdp->popup, "default", pdp->box);
	pdp->keep_button = elm_button_add(pdp->popup);
	elm_object_style_set(pdp->keep_button, "elm.swallow.content.button1");
	elm_object_text_set(pdp->keep_button, dgettext(SERVICE_NAME, "SID_BTN_OCSP_KEEP_APP"));
	elm_object_part_content_set(pdp->popup, "button1", pdp->keep_button);
	evas_object_smart_callback_add(pdp->keep_button, "clicked", keep_answer, pdp);
	pdp->uninstall_button = elm_button_add(pdp->popup);
	elm_object_style_set(pdp->uninstall_button, "elm.swallow.content.button2");
	elm_object_text_set(pdp->uninstall_button, dgettext(SERVICE_NAME, "SID_BTN_OCSP_UNINSTALL_APP"));
	elm_object_part_content_set(pdp->popup, "button2  ", pdp->uninstall_button);
	evas_object_smart_callback_add(pdp->uninstall_button, "clicked", uninstall_answer, pdp);
	evas_object_show(pdp->popup);
	// Showing the popup window
	evas_object_show(pdp->win);
	// Run the efl mainloop
	elm_run();
	// Shutdown elementary
	LogDebug("elm_shutdown()");
	elm_shutdown();
}

static int wait_for_parent_info(int pipe_in)
{
	// wait for parameters from pipe_in
	// timeout is set for 10 seconds
	struct timeval timeout = {10L, 0L};
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(pipe_in, &readfds);
	int sel = select(pipe_in + 1, &readfds, NULL, NULL, &timeout);

	if (sel == -1) {
		int error = errno;
		LogError("Cannot get info from parent. Exit popup");
		LogError("Error: " << CCHECKER::GetErrnoString(error));
		close(pipe_in);
		return -1;
	} else if (sel == 0) {
		LogError("Timeout reached! Exit popup - ERROR");
		close(pipe_in);
		return -1;
	}

	return 0;
}

void deserialize(cert_checker_popup_data *pdp, char *line, ssize_t line_length)
{
	BinaryStream stream;
	stream.Write(line_length, static_cast <void *>(line));
	std::string app_id;
	std::string pkg_id;
	LogDebug("------- Deserialization -------");
	// Deserialization order:
	// app_id, pkg_id
	CCHECKER::Deserialization::Deserialize(stream, app_id);
	LogDebug("app_id : " << app_id.c_str());
	pdp->app_id = app_id.c_str();
	CCHECKER::Deserialization::Deserialize(stream, pkg_id);
	LogDebug("pkg_id : " << pkg_id.c_str());
	pdp->pkg_id = pkg_id.c_str();
}

} // anonymous

EAPI_MAIN int
elm_main(int argc, char **argv)
{
	try {
		// int pipe_in and int pipe_out should be passed to Popup via args.
		// These parameters should be passed to Popup via pipe_in:
		// std::string    app_id
		// std::string    pkg_id
		struct cert_checker_popup_data pd;
		struct cert_checker_popup_data *pdp = &pd;
		LogDebug("############################ popup binary ################################");
		setlocale(LC_ALL, "");

		if (argc < 3) {
			LogError("To few args passed in exec to popup-bin - should be at least 3:");
			LogError("(binary-name, pipe_in, pipe_out)");
			LogError("return ERROR");
			return popup_status::EXIT_ERROR;
		}

		LogDebug("Passed args: " << argv[0] << ", " << argv[1] << ", " << argv[2]);
		int pipe_in;
		int pipe_out;

		// Parsing args (pipe_in, pipe_out)
		if (0 == sscanf(argv[1], "%d", &pipe_in)) {
			LogError("Error while parsing pipe_in; return ERROR");
			return popup_status::EXIT_ERROR;
		}

		if (pipe_in < 0 || pipe_in > FD_SETSIZE) {
			LogError("fb about pipe_in is in invalid.");
			return popup_status::EXIT_ERROR;
		}

		if (0 == sscanf(argv[2], "%d", &pipe_out) || pipe_out == -1) {
			LogError("Error while parsing pipe_out; return ERROR");
			return popup_status::EXIT_ERROR;
		}

		if (pipe_out < 0 || pipe_out > FD_SETSIZE) {
			LogError("fb about pipe_out is in invalid.");
			close(pipe_in);
			return popup_status::EXIT_ERROR;
		}

		LogDebug("Parsed pipes: IN: " << pipe_in << ", OUT: " <<  pipe_out);

		if (wait_for_parent_info(pipe_in) == -1) {
			close(pipe_out);
			return popup_status::EXIT_ERROR;
		}

		int  buff_size = 1024;
		char line[buff_size];
		ssize_t count = 0;

		do {
			count = TEMP_FAILURE_RETRY(read(pipe_in, line, buff_size));
		} while (0 == count);

		if (count < 0) {
			int error = errno;
			close(pipe_in);
			close(pipe_out);
			LogError("read returned a negative value (" << count << ")");
			LogError("error: " << CCHECKER::GetErrnoString(error));
			LogError("Exit popup - ERROR");
			return popup_status::EXIT_ERROR;
		}

		LogDebug("Read bytes : " << count);
		close(pipe_in); // cleanup
		deserialize(pdp, line, count);
		pdp->result = response_e::RESPONSE_ERROR;
		show_popup(pdp); // Showing popup
		// sending validation_result to popup-runner
		BinaryStream stream_out;
		LogDebug("pdp->result : " << pdp->result);
		CCHECKER::Serialization::Serialize(stream_out, pdp->result);

		if (-1 == TEMP_FAILURE_RETRY(write(pipe_out, stream_out.char_pointer(), stream_out.size()))) {
			LogError("Write to pipe failed!");
			close(pipe_out);
			return popup_status::EXIT_ERROR;
		}

		close(pipe_out);
		LogDebug("############################ /popup binary ################################");
		LogDebug("Return: " << popup_status::NO_ERROR);
		return popup_status::NO_ERROR;
	} catch (const CCHECKER::Exception &e) {
		LogError("Exception occured in popup main.");
		return popup_status::EXIT_ERROR;
	} catch (const std::exception &e) {
		LogError("Exception occured in popup main : " << e.what());
		return popup_status::EXIT_ERROR;
	} catch (...) {
		LogError("Unhandled exception occured in popup main.");
		return popup_status::EXIT_ERROR;
	}
}
ELM_MAIN()
