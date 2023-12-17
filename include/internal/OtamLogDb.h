#ifndef OTAM_LOG_DB_H
#define OTAM_LOG_DB_H

#include "internal/OtamHttp.h"

class OtamLogDb {
   private:
    String url;

   public:
    OtamLogDb(String url);

    void debug(String message);
    void info(String message);
    void error(String message);
};

#endif  // OTAM_LOG_DB_H