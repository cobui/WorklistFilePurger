//1) Move logic into class
//2) Reference the class into the plugin to instantiate class in Plugin.cpp
//3) Adjust CMake -> reference new files
//4) Adjust unit test accordingly


#include <orthanc/OrthancCPlugin.h>
#include "../../Common/OrthancPluginCppWrapper.h"
#include "Purger.h"


#include <boost/filesystem.hpp>
#include <json/value.h>
#include <json/reader.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <regex>

namespace OrthancPlugins {
    WorklistPurger::WorklistPurger(OrthancPluginContext*  context_, std::string folder_) {
        setFolder_(folder_);
        setContext_(context_);
    };
    WorklistPurger::~WorklistPurger() {};

    void WorklistPurger::setFolder_(std::string folder__) {
        folder_ = folder__;
        //std::cout << "Folder: " << folder_ << "\n";
    };

    void WorklistPurger::setContext_(OrthancPluginContext*  context__) {
        context_ = context__;
        //std::cout << "Context: " << context_ << "\n";
    };

    OrthancPluginErrorCode WorklistPurger::getStudyUID(const char* resourceId, std::string *result) {
        char info[1024];
        OrthancPluginMemoryBuffer answerBody;
        std::string uri = std::string ("/studies/") + std::string (resourceId) ;

        OrthancPluginRestApiGet(context_, &answerBody, uri.c_str());

        // Parse JSON
        Json::Value studyJSON;
        char *begin, *end;
        begin = (char*)answerBody.data;
        std::string s((const char*)answerBody.data);
        end = begin + (s.length() - 1);

        Json::CharReaderBuilder builder {};

        auto reader = std::unique_ptr<Json::CharReader>( builder.newCharReader() );

        Json::Value parsed_study {};
        std::string errors {};
        const auto is_parsed = reader->parse(begin,end,&parsed_study,&errors );
        OrthancPluginFreeMemoryBuffer(context_, &answerBody);

        if ( is_parsed != 0 )
        {
            OrthancPluginLogError(context_, "Parsing API response was not successful");
            return OrthancPluginErrorCode_BadJson;
        }

        try {
            std::string studyUID = parsed_study["MainDicomTags"]["StudyInstanceUID"].asString();
            sprintf(info, "Study Instance UID of the stable study: %s", studyUID.c_str());
            OrthancPluginLogInfo(context_, info);
            *result = studyUID;
            return OrthancPluginErrorCode_Success;
        }
        catch ( const Json::Exception& error )
        {
            sprintf(info, "Accessing Study Instance UID failed: %s", error.what());
            OrthancPluginLogError(context_, info);
            return OrthancPluginErrorCode_InexistentTag;
        }
    };

    void WorklistPurger::verifyAndRemoveWorklistFile(std::string studyUID) {
        namespace fs = boost::filesystem;
        fs::path source(folder_);
        fs::directory_iterator end;

        for (fs::directory_iterator it(source); it != end; ++it) {
            std::string filename = it->path().filename().string();
            std::string match_string = "(worklist_" + studyUID + "_)(.*)";

            if (std::regex_match(filename, std::regex(match_string))) {
                std::string log_info = "Found a matching worklist file file for Study Instance UID " + studyUID;
                OrthancPluginLogWarning(context_, log_info.c_str());

                if (remove(it->path())) {
                    std::string log_info = "Worklist file was successfully deleted";
                    OrthancPluginLogWarning(context_, log_info.c_str());
                } else {
                    std::string log_info = "Unable to delete worklist file";
                    OrthancPluginLogError(context_, log_info.c_str());
                };

                return;
            }
        }
        std::string log_info = "No worklist file for Study Instance UID " + studyUID + " found";
        OrthancPluginLogWarning(context_, log_info.c_str());

        return;
    };

    OrthancPluginErrorCode WorklistPurger::OnChangeCallback(OrthancPluginChangeType changeType,OrthancPluginResourceType
                                                                           resourceType, const char* resourceId) {
        char info[1024];
        if (changeType == OrthancPluginChangeType_StableStudy ) {
            std::string studyUID;
            getStudyUID(resourceId, &studyUID);

            verifyAndRemoveWorklistFile(studyUID);
        }

        return OrthancPluginErrorCode_Success;
    }
}
