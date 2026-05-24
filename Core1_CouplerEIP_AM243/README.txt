Add protocol macro
-DACTIVE_PROTOCOL=1

Add debug memory trace macro
-DCMN_MEM_TRACE=1

-I"${workspace_loc:/${ProjName}/common/inc}"
-I"${workspace_loc:/${ProjName}/config}" 
-I"${workspace_loc:/${ProjName}/config/board/am243x-lp}"
-I"${workspace_loc:/${ProjName}/device_profiles}"
-I"${workspace_loc:/${ProjName}/device_profiles/common}" 
-I"${workspace_loc:/${ProjName}/device_profiles/generic_device}"
-I"${workspace_loc:/${ProjName}/os/freertos}"
-I"${workspace_loc:/${ProjName}/custom}"
-I"${workspace_loc:/${ProjName}/drivers}"
-I"${workspace_loc:/${ProjName}/canopenmaster/inc}" 
-I"${workspace_loc:/${ProjName}/coupler_master}" 
-I"${workspace_loc:/${ProjName}}"  
-I"${workspace_loc:/${ProjName}/driver}" 
-I"${workspace_loc:/${ProjName}/webserver}"