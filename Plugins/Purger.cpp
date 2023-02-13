//1) Move logic into class
//2) Reference the class into the plugin to instantiate class in Plugin.cpp
//3) Adjust CMake -> reference new files
//4) Adjust unit test accordingly


#include <orthanc/OrthancCPlugin.h>
#include "../../Common/OrthancPluginCppWrapper.h"
#include "Purger.h"
#include "/usr/local/include/orthanc_sources/Logging.h"

#include <boost/filesystem.hpp>
#include <json/value.h>
#include <json/reader.h>
#include <iostream>
#include <regex>


namespace OrthancPlugins {
    WorklistPurger::WorklistPurger(OrthancPluginContext*  context__, std::string folder__) {
        namespace fs = std::filesystem;
        try {
            std::error_code ec;
            if (fs::is_directory(fs::path(folder__), ec)) {
                folder_ = folder__;
            } else {
                std::cerr << "Error: " << ec << "\n";
                throw std::runtime_error("Worklist folder does not exist - check configuration file");
            };
        }catch(std::runtime_error e) {
            std::cerr << e.what() << "\n";
        }
        context_ = context__;
    };
    WorklistPurger::~WorklistPurger() {};

    OrthancPluginErrorCode WorklistPurger::getStudyUID(const char* resourceId, std::string *result) {
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
            LOG(ERROR) << "Parsing API response was not successful";
            return OrthancPluginErrorCode_BadJson;
        }

        try {
            std::string studyUID = parsed_study["MainDicomTags"]["StudyInstanceUID"].asString();
            LOG(INFO) << "Study Instance UID of the stable study: %s" << studyUID;
            *result = studyUID;
            return OrthancPluginErrorCode_Success;
        }
        catch ( const Json::Exception& error )
        {
            LOG(ERROR) << "Accessing Study Instance UID failed: %s" << error.what();
            return OrthancPluginErrorCode_InexistentTag;
        }
    };

    void WorklistPurger::verifyAndRemoveWorklistFile(std::string studyUID) {
        namespace fs = boost::filesystem;
        fs::path source(folder_);
        fs::directory_iterator end;
        std::string escaped_studyUID;

        for (auto it = studyUID.cbegin() ; it != studyUID.cend(); ++it) {
            if (*it == '.') {
                escaped_studyUID += "\\";
            }
            escaped_studyUID += *it;
        }

        for (fs::directory_iterator it(source); it != end; ++it) {
            std::string filename = it->path().filename().string();
            std::string match_string = "(worklist_" + escaped_studyUID + "_)(.*)";

            if (std::regex_match(filename, std::regex(match_string))) {
                LOG(WARNING) << "Found a matching worklist file file for Study Instance UID " << studyUID;
                if (remove(it->path())) {
                    LOG(WARNING) << "Worklist file was successfully deleted";
                } else {
                    LOG(ERROR) << "Unable to delete worklist file";
                };
                return;
            }
        }
        LOG(WARNING) << "No worklist file for Study Instance UID " << studyUID << " found";
        return;
    };

    OrthancPluginErrorCode WorklistPurger:: OnChangeCallback(OrthancPluginChangeType changeType,OrthancPluginResourceType
                                                                           resourceType, const char* resourceId) {
        if (changeType == OrthancPluginChangeType_StableStudy ) {
            std::string studyUID;
            getStudyUID(resourceId, &studyUID);

            verifyAndRemoveWorklistFile(studyUID);
        }

        return OrthancPluginErrorCode_Success;
    }
}
