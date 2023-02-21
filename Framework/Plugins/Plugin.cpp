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
#include "../../Resources/Orthanc/Plugins/OrthancPluginCppWrapper.h"
#include "/usr/local/include/orthanc_sources/Logging.h"
//#include <Logging.h>
#include <filesystem>

static std::string folder_;
static OrthancPlugins::WorklistPurger* purger_;

namespace fs = std::filesystem;

OrthancPluginErrorCode OnChange(OrthancPluginChangeType changeType,OrthancPluginResourceType resourceType, const char*
                                                                                                           resourceId) {
    purger_->OnChangeCallback(changeType, resourceType, resourceId);
    return OrthancPluginErrorCode_Success;
};

extern "C" {
    ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* c) {
        OrthancPluginContext* context_ = c;
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
        OrthancPlugins::OrthancConfiguration worklistPurger;

        configuration.GetSection(worklistsConf, "Worklists");
        configuration.GetSection(worklistPurger, "WorklistPurger");

        bool serverEnabled = worklistsConf.GetBooleanValue("Enable", false);
        bool pluginEnabled = worklistPurger.GetBooleanValue("Enable", false);

        worklistsConf.LookupStringValue(folder_, "Database");

        LOG(WARNING) << "WorklistPurger: " << pluginEnabled;

        if (serverEnabled && pluginEnabled && fs::is_directory(fs::path(folder_))) {
                try {
                    purger_ = new OrthancPlugins::WorklistPurger(context_, folder_);
                    OrthancPluginRegisterOnChangeCallback(context_, OnChange);
                } catch (std::exception& e) {
                    LOG(ERROR) << e.what();
                    return -1;
                }
        } else {
          LOG(WARNING) << "Worklist server or WorklistPurger plugin is disabled in the Orthanc configuration file";
          LOG(WARNING) << "The configuration option \"Worklists.Database\" must contain a path to an existing folder in the Orthanc configuration file";
        }
        return 0;
    }


  ORTHANC_PLUGINS_API void OrthancPluginFinalize()
  {
    LOG(WARNING) << "WorklistPurger plugin is finalizing";
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

  


