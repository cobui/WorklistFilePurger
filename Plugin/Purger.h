#include <orthanc/OrthancCPlugin.h>
#include "../Resources/Orthanc/Plugins/OrthancPluginCppWrapper.h"

namespace OrthancPlugins {
    class WorklistPurger {
        private:
            OrthancPluginContext*  context_;
            std::string folder_;
        public:
            WorklistPurger(OrthancPluginContext*  context_, std::string folder_);
            virtual ~WorklistPurger();
            virtual OrthancPluginErrorCode getStudyUID(const char* resourceId, std::string *result);
            virtual void verifyAndRemoveWorklistFile(std::string* studyUID);
            virtual void escapeDotsInFilename(std::string* studyUID, std::string* escaped_studyUID);
            virtual OrthancPluginErrorCode OnChangeCallback(OrthancPluginChangeType changeType,OrthancPluginResourceType
                                                                                  resourceType, const char* resourceId);
    };
};