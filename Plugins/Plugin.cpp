/**
 * Orthanc - A Lightweight, RESTful DICOM Store
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2020 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include "Purger.h"
#include <orthanc/OrthancCPlugin.h>
#include "../../Common/OrthancPluginCppWrapper.h"

#include <boost/filesystem.hpp>
#include <json/value.h>
#include <json/reader.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <regex>

#define DCM_TAG_STDIUID "0020,000d"
#define DCM_TAG_ACCNO "0008,0050"
#define WORKLIST_PURGE_CACHE "WorklistPurgeCache"

static std::string folder_;
static OrthancPluginContext* context_ = NULL;
static OrthancPlugins::WorklistPurger* purger_;

bool worklistPurgerRESTStatus;

// OrthancPluginErrorCode getStudyUID(const char* resourceId, std::string *result) {
//     char info[1024];
//     OrthancPluginContext *context = OrthancPlugins::GetGlobalContext();
//     OrthancPluginMemoryBuffer answerBody;
//     const char* orthancUrl = "/studies/";
//     char* uri;
//     uri = (char*) calloc(strlen(orthancUrl) + strlen(resourceId) + 1, sizeof(char));
//     strcpy(uri, orthancUrl);
//     strcat(uri, resourceId);
//
//     OrthancPluginRestApiGet(context, &answerBody, uri);
//
//     // Parse JSON
//     Json::Value studyJSON;
//     char *begin, *end;
//     begin = (char*)answerBody.data;
//     std::string s((const char*)answerBody.data);
//     end = begin + (s.length() - 1);
//
//     Json::CharReaderBuilder builder {};
//
//     auto reader = std::unique_ptr<Json::CharReader>( builder.newCharReader() );
//
//     Json::Value parsed_study {};
//     std::string errors {};
//     const auto is_parsed = reader->parse(begin,end,&parsed_study,&errors );
//     OrthancPluginFreeMemoryBuffer(context, &answerBody);
//
//     if ( is_parsed != 0 )
//     {
//         OrthancPluginLogError(OrthancPlugins::GetGlobalContext(), "Parsing API response was not successful");
//         return OrthancPluginErrorCode_BadJson;
//     }
//
//     try {
//         std::string studyUID = parsed_study["MainDicomTags"]["StudyInstanceUID"].asString();
//         sprintf(info, "Study Instance UID of the stable study: %s", studyUID.c_str());
//         OrthancPluginLogInfo(OrthancPlugins::GetGlobalContext(), info);
//         *result = studyUID;
//         return OrthancPluginErrorCode_Success;
//     }
//     catch ( const Json::Exception& error )
//     {
//         sprintf(info, "Accessing Study Instance UID failed: %s", error.what());
//         OrthancPluginLogError(OrthancPlugins::GetGlobalContext(), info);
//         return OrthancPluginErrorCode_InexistentTag;
//     }
// }
//
// void verifyAndRemoveWorklistFile(std::string studyUID) {
//   namespace fs = boost::filesystem;
//   fs::path source(folder_);
//   fs::directory_iterator end;
//   std::cout << folder_ << "\n";
//   for (fs::directory_iterator it(source); it != end; ++it) {
//     std::string filename = it->path().filename().string();
//     std::string match_string = "(worklist_" + studyUID + "_)(.*)";
//     //std::cout << it->path().filename().string() << "\n";
//
//     if (std::regex_match(filename, std::regex(match_string))) {
//         std::string log_info = "Found a matching worklist file file for Study Instance UID " + studyUID;
//         //std::cout << "Found a matching worklist file file for Study Instance UID " << "\n";
//         std::cout << context_ << "\n";
//         OrthancPluginLogWarning(context_, log_info.c_str());
//
//         if (remove(it->path())) {
//             std::string log_info = "Worklist file was successfully deleted";
//             //std::cout << "Worklist file was successfully deleted" << "\n";
//             //OrthancPluginLogWarning(OrthancPlugins::GetGlobalContext(), log_info.c_str());
//         } else {
//             std::string log_info = "Unable to delete worklist file";
//             //std::cout << "Unable to delete worklist file" << "\n";
//             //OrthancPluginLogError(OrthancPlugins::GetGlobalContext(), log_info.c_str());
//         };
//
//         return;
//     }
//   }
//   std::string log_info = "No worklist file for Study Instance UID " + studyUID + " found";
//   //OrthancPluginLogWarning(OrthancPlugins::GetGlobalContext(), log_info.c_str());
//
//   return;
// }

// OrthancPluginErrorCode OnChangeCallback(OrthancPluginChangeType changeType,OrthancPluginResourceType resourceType,
//                                              const char* resourceId)
// {
//   char info[1024];
//   if (changeType == OrthancPluginChangeType_StableStudy ) {
//     std::string studyUID;
//     getStudyUID(resourceId, &studyUID);
//
//     verifyAndRemoveWorklistFile(studyUID);
//   }
//
//   return OrthancPluginErrorCode_Success;
// }

OrthancPluginErrorCode OnChange(OrthancPluginChangeType changeType,OrthancPluginResourceType resourceType, const char*
                                                                                                           resourceId) {
    purger_->OnChangeCallback(changeType, resourceType, resourceId);
    return OrthancPluginErrorCode_Success;
};

extern "C" {
    ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* c) {
        OrthancPlugins::SetGlobalContext(c);
        context_ = c;

        /* Check the version of the Orthanc core */
        if (OrthancPluginCheckVersion(context_) == 0) {
            char info[1024];
            sprintf(info, "Your version of Orthanc (%s) must be above %d.%d.%d to run this plugin",
                  OrthancPlugins::GetGlobalContext()->orthancVersion,
                  ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER,
                  ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER,
                  ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER);
            OrthancPluginLogError(context_, info);
        return -1;
        }

        worklistPurgerRESTStatus = true;

        OrthancPlugins::LogWarning("WorklistFilePurger :  plugin is initializing");
        OrthancPluginSetDescription(c, "Delete worklist file after receiving a matching instance in Orthanc PACS");


        OrthancPlugins::OrthancConfiguration configuration;

        OrthancPlugins::OrthancConfiguration worklistsConf;
        configuration.GetSection(worklistsConf, "Worklists");


        bool enabled = worklistsConf.GetBooleanValue("Enable", false);

        if (enabled) {
        if (worklistsConf.LookupStringValue(folder_, "Database")) {
            //Registering Callbacks
            try {
                /* Create the purger instance */
                purger_ = new OrthancPlugins::WorklistPurger(context_, folder_);
                OrthancPluginRegisterOnChangeCallback(context_, OnChange);
            } catch (std::exception& e) {
                OrthancPluginLogError(context_, e.what());
                return -1;
            }
//         OrthancPluginRegisterOnChangeCallback(context_, OnChangeCallback);
//         OrthancPluginRegisterRestCallback(OrthancPlugins::GetGlobalContext(), "/enableWorklistPurge", EnableWorklistPurger);
//         OrthancPluginRegisterRestCallback(OrthancPlugins::GetGlobalContext(), "/disableWorklistPurge", DisableWorklistPurger);
//         OrthancPluginRegisterRestCallback(OrthancPlugins::GetGlobalContext(), "/worklistPurgeStatus", WorklistPurgerStatus);
        return 0;
        } else {
            OrthancPlugins::LogError("WorklistFilePurger : The configuration option \"Worklists.Database\" must contain a path in the ORthanc configuration file");
            return -1;
        }

    }
    else
    {
      OrthancPlugins::LogWarning("WorklistFilePurger : Worklist server is disabled in the ORthanc configuration file");
    }

    return 0;

  }


  ORTHANC_PLUGINS_API void OrthancPluginFinalize()
  {
    OrthancPlugins::LogWarning("WorklistFilePurger plugin is finalizing");
  }


  ORTHANC_PLUGINS_API const char* OrthancPluginGetName()
  {
    return "worklist-file-purger";
  }
	


  ORTHANC_PLUGINS_API const char* OrthancPluginGetVersion()
  {
    return WORKLIST_FILE_PURGER_VERSION;
  }
}

  


