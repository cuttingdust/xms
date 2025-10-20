/**
 * @file   XUploadService.cpp
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-20
 */

#ifndef XUPLOADSERVICE_CPP
#define XUPLOADSERVICE_CPP

#include <XService.h>

class XUploadService : public XService
{
public:
    XUploadService();
    ~XUploadService() override;

public:
    auto main(int argc, char *argv[]) -> void;

    auto createHandle() -> XServiceHandle * override;
};


#endif // XUPLOADSERVICE_CPP
