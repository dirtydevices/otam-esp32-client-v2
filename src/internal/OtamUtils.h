#include <Arduino.h>

class OtamUtils
{
public:
    static String generatePseudoGUID()
    {
        String guid = "otam_";
        static const char alphanum[] = "0123456789ABCDEF";

        for (int i = 0; i < 8; i++)
        {
            guid += alphanum[random(16)];
        }
        guid += "-";
        for (int i = 0; i < 4; i++)
        {
            guid += alphanum[random(16)];
        }
        guid += "-";
        for (int i = 0; i < 4; i++)
        {
            guid += alphanum[random(16)];
        }

        return guid;
    }
};
