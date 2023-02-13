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
#include "/usr/local/include/orthanc_sources/Logging.h"

#define DCM_TAG_STDIUID "0020,000d"
#define DCM_TAG_ACCNO "0008,0050"
#define WORKLIST_PURGE_CACHE "WorklistPurgeCache"

static std::string folder_;
static OrthancPluginContext* context_ = NULL;
static OrthancPlugins::WorklistPurger* purger_;


OrthancPluginErrorCode OnChange(OrthancPluginChangeType changeType,OrthancPluginResourceType resourceType, const char*
                                                                                                           resourceId) {
    purger_->OnChangeCallback(changeType, resourceType, resourceId);
    return OrthancPluginErrorCode_Success;
};

extern "C" {
    ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* c) {
        context_ = c;
        OrthancPlugins::SetGlobalContext(context_);
        Orthanc::Logging::InitializePluginContext(context_);

        /* Check the version of the Orthanc core */
        if (OrthancPluginCheckVersion(context_) == 0) {
            LOG(ERROR) << "Your version of Orthanc" <<( OrthancPlugins::GetGlobalContext()->orthancVersion)
                       <<  "must be above "
                       <<  ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER <<"." <<ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER <<"."
                       << ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER << "to run this plugin";
        return -1;
        }

        LOG(WARNING) << "WorklistFilePurger :  plugin is initializing";
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
                LOG(ERROR) << e.what();
                return -1;
            }
        return 0;
        } else {
            LOG(ERROR) << "WorklistFilePurger : The configuration option \"Worklists.Database\" must contain a path in the ORthanc configuration file";
            return -1;
        }

    }
    else
    {
      LOG(WARNING) << "WorklistFilePurger : Worklist server is disabled in the ORthanc configuration file";
    }

    return 0;

  }


  ORTHANC_PLUGINS_API void OrthancPluginFinalize()
  {
    LOG(WARNING) << "WorklistFilePurger plugin is finalizing";
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

  


