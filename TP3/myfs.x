const MYFS_MAX_NAME_LENGTH = 256;
const LOCK_WAITING = 0;
const LOCK_GRANTED = 1;
const FILE_ERROR = -1;
const FILE_OK = 1;

struct LockRequest {
    string lock_name<MYFS_MAX_NAME_LENGTH>;
    string client_id<MYFS_MAX_NAME_LENGTH>;
};

struct FileRequest {
    string file_name<MYFS_MAX_NAME_LENGTH>;
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
        int CANCEL_LOCK(LockRequest) = 6;
    } = 1;
} = 0x31111111;