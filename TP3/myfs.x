/* myfs.x */

struct LockRequest {
    string lock_name<256>;
    string client_id<256>;
};

struct FileRequest {
    string file_name<256>;
};

struct FileResponse {
    int status; 
    opaque data<>; 
};

program MYFS_PROG {
    version MYFS_VERS {
        
        int ACQUIRE_LOCK(LockRequest) = 1;
        int CHECK_LOCK(LockRequest) = 2;
        int RELEASE_LOCK(LockRequest) = 3;
        
        
        string GET_SERVER_STATUS(void) = 4;
        
        
        FileResponse GET_FILE(FileRequest) = 5;
    } = 1;
} = 0x31111111;