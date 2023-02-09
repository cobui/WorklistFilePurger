#include <orthanc/OrthancCPlugin.h>
#include "../../Common/OrthancPluginCppWrapper.h"

namespace OrthancPlugins {
    class WorklistPurger {
        private:
            OrthancPluginContext*  context_;
            std::string folder_;
        public:
            WorklistPurger(OrthancPluginContext*  context_, std::string folder_);
            virtual ~WorklistPurger();
            virtual void setContext_(OrthancPluginContext*  context__);
            virtual void setFolder_(std::string folder__);
            virtual OrthancPluginErrorCode getStudyUID(const char* resourceId, std::string *result);
            virtual void verifyAndRemoveWorklistFile(std::string studyUID);
            virtual OrthancPluginErrorCode OnChangeCallback(OrthancPluginChangeType changeType,OrthancPluginResourceType
                                                                                  resourceType, const char* resourceId);
    };
};