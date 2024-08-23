/*
*   This file is part of Universal-Updater
*   Copyright (C) 2019-2021 Universal-Team
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/

#include "common.hpp"
#include "fileBrowse.hpp"
#include "meta.hpp"
#include <unistd.h>

/*
	The Constructor of the Meta.

	Includes MetaData file creation, if non existent.
*/
Meta::Meta() {
	if (access(_META_PATH, F_OK) != 0) {
		FILE *temp = fopen(_META_PATH, "w");
		char tmp[2] = { '{', '}' };
		fwrite(tmp, sizeof(tmp), 1, temp);
		fclose(temp);
	}

	FILE *temp = fopen(_META_PATH, "rt");
	if (temp) {
		this->metadataJson = nlohmann::json::parse(temp, nullptr, false);
		fclose(temp);
	}
	if (this->metadataJson.is_discarded())
		this->metadataJson = { };

	if (config->metadata()) this->ImportMetadata();
}

/*
	Import the old Metadata of the 'updates.json' file.
*/
void Meta::ImportMetadata() {
	if (access("sdmc:/3ds/Universal-Updater/updates.json", F_OK) != 0) {
		config->metadata(false);
		return; // Not found.
	}

	Msg::DisplayMsg(Lang::get("FETCHING_METADATA"));

	nlohmann::json oldJson;
	FILE *old = fopen("sdmc:/3ds/Universal-Updater/updates.json", "rt");
	if (old) {
		oldJson = nlohmann::json::parse(old, nullptr, false);
		fclose(old);
	}
	if (oldJson.is_discarded())
		oldJson = { };

	std::vector<StoreInfo> info = GetStoreInfo(_STORE_PATH);

	for (int i = 0; i < (int)info.size(); i++) {
		if (info[i].Title != "" && oldJson.contains(info[i].FileName)) {
			for(auto it = oldJson[info[i].FileName].begin(); it != oldJson[info[i].FileName].end(); ++it) {
				this->SetUpdated(info[i].Title, it.key().c_str(), it.value().get<std::string>());
			}
		}
	}

	config->metadata(false);
}


std::string Meta::GetUpdated(const std::string &storeName, const std::string &entry) const {
	if (!this->metadataJson.contains(storeName)) return "";

	if (!this->metadataJson[storeName].contains(entry)) return ""; // Entry does not exist.

	if (!this->metadataJson[storeName][entry].contains("updated")) return ""; // updated does not exist.

	if (this->metadataJson[storeName][entry]["updated"].is_string()) return this->metadataJson[storeName][entry]["updated"];
	return "";
}

int Meta::GetMarks(const std::string &storeName, const std::string &entry) const {
	int temp = 0;

	if (!this->metadataJson.contains(storeName)) return temp;

	if (!this->metadataJson[storeName].contains(entry)) return temp; // Entry does not exist.

	if (!this->metadataJson[storeName][entry].contains("marks")) return temp; // marks does not exist.

	if (this->metadataJson[storeName][entry]["marks"].is_number()) return this->metadataJson[storeName][entry]["marks"];
	return temp;
}


bool Meta::UpdateAvailable(const std::string &storeName, const std::string &entry, const std::string &updated) const {
	if (this->GetUpdated(storeName, entry) != "" && updated != "") {
		return strcasecmp(updated.c_str(), this->GetUpdated(storeName, entry).c_str()) > 0;
	}

	return false;
}

std::vector<std::string> Meta::GetInstalled(const std::string &storeName, const std::string &entry) const {
	if (!this->metadataJson.contains(storeName)) return { };

	if (!this->metadataJson[storeName].contains(entry)) return { }; // Entry does not exist.

	if (!this->metadataJson[storeName][entry].contains("installed")) return { }; // marks does not exist.

	if (this->metadataJson[storeName][entry]["installed"].is_array()) return this->metadataJson[storeName][entry]["installed"];
	return { };
}


void Meta::SaveCall() {
	FILE *file = fopen(_META_PATH, "wb");
	const std::string dump = this->metadataJson.dump(1, '\t');
	fwrite(dump.c_str(), 1, dump.size(), file);
	fclose(file);
}