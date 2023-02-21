/**
 * Orthanc - A Lightweight, RESTful DICOM Store
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2022 Osimis S.A., Belgium
 * Copyright (C) 2021-2022 Sebastien Jodogne, ICTEAM UCLouvain, Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include <gtest/gtest.h>

#include "/usr/local/include/orthanc_sources/Logging.h"

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <filesystem>
#include "../Framework/Plugins/Purger.h"

void openAndStoreAsString(std::string* result, std::string file) {
        std::ifstream t(file);
        std::stringstream buffer;
        buffer << t.rdbuf();
        *result = buffer.str();
}


OrthancPluginErrorCode PluginServiceMock(struct _OrthancPluginContext_t* context,_OrthancPluginService service,
const void* params)
{
    if (service == _OrthancPluginService_RestApiGet)
    {
        const _OrthancPluginRestApiGet& p = *reinterpret_cast<const _OrthancPluginRestApiGet*>(params);
        std::string data;
        openAndStoreAsString(&data, "study.txt");

        size_t size = data.size();
        p.target->data = malloc(size);
        memcpy(p.target->data, data.c_str(), size);
    }
    return OrthancPluginErrorCode_Success;
}

OrthancPluginErrorCode logServiceMock(struct _OrthancPluginContext_t* context,_OrthancPluginService service,
const void* params)
{
    return OrthancPluginErrorCode_Success;
}

class WorklistRemoverTest : public ::testing::Test {
    protected:
        std::unique_ptr<OrthancPluginContext> context_;
        OrthancPluginContext pluginContext_;
        std::unique_ptr<OrthancPlugins::WorklistPurger> purger_;
        std::string folder_;

    virtual void SetUp() {
        namespace fs = std::filesystem;
        context_ = std::make_unique<OrthancPluginContext>();
        context_->InvokeService = &PluginServiceMock;
        context_->Free = &free;
        //making unique pointer throws error
        //no viable conversion from 'std::unique_ptr<OrthancPluginContext>' (aka 'unique_ptr<_OrthancPluginContext_t>') to 'void *'
        pluginContext_.InvokeService = &logServiceMock;
        Orthanc::Logging::InitializePluginContext(&pluginContext_);


        //create a directory for dummy files
        fs::path p = fs::current_path() ;
        p.concat("/worklist_files");
        std::error_code ec;

        if (fs::create_directory(p, ec)) {
            folder_ = p.generic_string();
            purger_ = std::make_unique<OrthancPlugins::WorklistPurger>(context_.get(), folder_);
        } else {
            std::cerr << "Error Code: "<< ec << "\nCreating worklist_files testfolder was not successful" << "\n";
            return;
        };


        //create worklist files
        for (int i = 0; i < 10; i++) {
            std::string string_i = std::to_string(i);
            std::string filename = folder_ + "/worklist_12.234.1234.23456" + string_i + "_unixtime.wl";
            const char* full_file_path = filename.c_str();
            FILE* file_pointer = std::fopen(full_file_path, "w");
        }
    }

    virtual void TearDown() {
        // remove worklist files and directory
        std::filesystem::remove_all(folder_);
        std::filesystem::remove(folder_);
    }

};

TEST_F(WorklistRemoverTest, getStudyUID) {
    const char* uri = "/studies/27f7126f-4f66fb14-03f4081b-f9341db2-53925988";

    const char* resourceId = "27f7126f-4f66fb14-03f4081b-f9341db2-53925988";
    std::string result;
    ASSERT_EQ(purger_->getStudyUID(resourceId, &result), OrthancPluginErrorCode_Success);
}

TEST_F(WorklistRemoverTest, verifyAndRemoveWorklist) {
    std::string fileName;

    for (int i = 1; i < 10; i+=2) {
        std::string string_i = std::to_string(i);
        std::string studyUID = "12.234.1234.23456" + string_i;
        std::string filename = "worklist_" + studyUID + "_unixtime.wl";
        std::string full_file_path = folder_ + "/" + filename;

        const char* file = full_file_path.c_str();
        FILE* file_pointer = std::fopen(file, "r");

        ASSERT_TRUE(file_pointer != NULL);
        fclose(file_pointer);
        purger_->verifyAndRemoveWorklistFile(&studyUID);
        std::ifstream ifile;
        ifile.open(file);
        ASSERT_FALSE(ifile);
    }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}