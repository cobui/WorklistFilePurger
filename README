### Modality Worklist Purger plugins for Orthanc DICOM Server
An Orthanc Plugin for clearing the Modality Worklist files, once the instance received in Orthanc PACS Server

#### OverView
When an Instance with a matching accession number or Study Instance UID received in Orthanc PACS, this plugin will remove the corresponding worklist entry from the worklist database folder.
The plugin reads the worklist database path from the "ModalityWorklists" section of the Orthanc Configuration, therefore it is mandatory to have this section available in the Orthanc Configuration file for the smooth working of the plugin.


#### Building the Plugin
Check out the code and copy it into the Orthanc Samples folder
```bash
cd OrthancServer/Plugins/Samples
git clone https://github.com/alvinak/WorklistFilePurger.git
cd WorklistFilePurger
mkdir Build
cd Build
cmake .. -DSTATIC_BUILD=ON -DCMAKE_BUILD_TYPE=Release
make
```

#### Callback functions used
```bash
OrthancPluginRegisterOnChangeCallback
```

#### Side note 1
This plugin identifies the worklist files based on a set naming scheme which is: "worklist_${studyUID}_(.*)". If your worklist files follow any other naming scheme, the plugin will not pick these files up and will not delete them!

#### Side note 2
The starting point of this plugin functionality is the OrthancPluginRegisterOnChangeCallback function by listening to the OnStableStuy event.

More ideal way is to use Modality Peroformed Procedure Step (MPPS) and it can be achieved by using RegisterOnChangeCallback function as per the below Orthanc group discussion.
https://groups.google.com/g/orthanc-users/c/Tec0xgD4s2c
There is a plan to implement RegisterOnChangeCallback in the future release of this plugin.

